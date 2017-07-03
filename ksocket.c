#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <unistd.h>
#include <uv.h>
#include <sys/un.h>
#include "list.h"
#include "ksocket.h"

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif /* MIN */
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif	/* MAX */

struct ks_buffer *ks_buffer_create()
{
    struct ks_buffer *buffer = calloc(1, sizeof(struct ks_buffer));
    
    buffer->totalsize = sizeof(buffer->data);
    
    return buffer;
}

void ks_buffer_destroy(struct ks_buffer *buffer)
{
    if(buffer->data2)
    {
        free(buffer->data2);
        buffer->data2 = NULL;
    }
    
    free(buffer);
}

int64_t ks_buffer_addref(struct ks_buffer *buffer)
{
    int64_t refcount;

    refcount = __sync_add_and_fetch(&buffer->refcount, 1);

    if(refcount == 1)
    {
        ks_buffer_reset(buffer);
    }

    return refcount;
}

kboolean ks_buffer_decref(struct ks_buffer *buffer)
{
    if(__sync_sub_and_fetch(&buffer->refcount, 1) <= 0)
    {
        return 1;
    }
    
    return 0;
}

void ks_buffer_write(struct ks_buffer *buffer, void *data, size_t size)
{
    size_t totalsize;
    
    
    totalsize = buffer->usingsize + size;
    
    if(buffer->data2)
    {
        if(totalsize > buffer->totalsize)
        {
            buffer->data2 = realloc(buffer->data2, align_size(totalsize, KS_BUFFER_DEFAULT_DATA_SIZE));
        }
        
        memmove(&buffer->data2[buffer->usingsize], data, size);
        buffer->usingsize = totalsize;
    }
    else
    {
        if(buffer->totalsize < totalsize)
        {
            buffer->data2 = malloc(align_size(totalsize, KS_BUFFER_DEFAULT_DATA_SIZE));
            memcpy(buffer->data2, buffer->data, buffer->usingsize);
            memcpy(&buffer->data2[buffer->usingsize], data, size);
            buffer->usingsize = totalsize;
        }
        else
        {
            memcpy(&buffer->data[buffer->usingsize], data, size);
            buffer->usingsize = totalsize;
        }
    }
}

void *ks_buffer_getdata(struct ks_buffer *buffer)
{
    if(buffer->data2)
    {
        return buffer->data2;
    }
    
    return buffer->data;
}

size_t ks_buffer_size(struct ks_buffer *buffer)
{
    return buffer->usingsize;
}

void ks_buffer_reset(struct ks_buffer *buffer)
{
    if(buffer->data2)
    {
        free(buffer->data2);
        buffer->data2 = NULL;
    }
    
    buffer->totalsize = sizeof(buffer->data);
    buffer->usingsize = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////



void INIT_KS_CIRCULAR_BUFFER(struct ks_circular_buffer *circular_buffer)
{
    INIT_LIST_HEAD(&circular_buffer->head);
    
    circular_buffer->inpos = 0;
    circular_buffer->outpos = 0;
    
    circular_buffer->totalsize = 0;
    circular_buffer->usingsize = 0;
    
    ks_circular_buffer_addblock(circular_buffer);
    
    circular_buffer->tail = circular_buffer->head.next;
    circular_buffer->first = circular_buffer->head.next;
}

void ks_circular_buffer_destroy(struct ks_circular_buffer *circular_buffer)
{
    struct ks_circular_buffer_block *buffer_block;
    
    circular_buffer->tail = NULL;
    
    while(list_empty(&circular_buffer->head))
    {
        buffer_block = list_first_entry(&circular_buffer->head, struct ks_circular_buffer_block, entry);
        list_del(&buffer_block->entry);
        free(buffer_block);
    }
    
    free(circular_buffer);
}

void ks_circular_buffer_queue(struct ks_circular_buffer *circular_buffer, const void *data, size_t size)
{
    struct ks_circular_buffer_block *buffer;
    size_t totalsize;
    size_t nread;
    size_t ncopy;
    const unsigned char *from;
    
    
    nread = 0;
    from = data;
    
    buffer = container_of(circular_buffer->tail, struct ks_circular_buffer_block, entry);

    while(nread != size)
    {
        totalsize = sizeof(buffer->buf) - circular_buffer->inpos;
        ncopy = MIN(totalsize, (size - nread));
        
        memcpy(&buffer->buf[circular_buffer->inpos], from, ncopy);
        
        from += ncopy;
        nread += ncopy;
        
        circular_buffer->usingsize += ncopy;
        circular_buffer->inpos += ncopy;
        if(circular_buffer->inpos == KS_CIRCULAR_BUFFER_BLOCK_SIZE)
        {
            circular_buffer->inpos = 0;
            
            if(list_is_last(&buffer->entry, &circular_buffer->head))
            {
                ks_circular_buffer_addblock(circular_buffer);
            }
            
            buffer = container_of(buffer->entry.next, struct ks_circular_buffer_block, entry);
        }
    }
    
    circular_buffer->tail = &buffer->entry;
}

void ks_circular_buffer_queue_ks_buffer(struct ks_circular_buffer *circular_buffer, struct ks_buffer *buffer)
{
    ks_circular_buffer_queue(circular_buffer, ks_buffer_getdata(buffer), ks_buffer_size(buffer));
}

kboolean ks_circular_buffer_peek_array(struct ks_circular_buffer *circular_buffer, void *data, size_t size)
{
    struct ks_circular_buffer_block *buffer;
    size_t nread;
    size_t cursor;
    size_t ncopy;
    unsigned char *to;
    
    if(circular_buffer->usingsize < size)
        return 0;
    
    nread = 0;
    cursor = circular_buffer->outpos;
    buffer = container_of(circular_buffer->first, struct ks_circular_buffer_block, entry);
    to = data;
    
    while(nread != size)
    {
        ncopy = MIN((KS_CIRCULAR_BUFFER_BLOCK_SIZE - cursor), (size - nread));
        memcpy(to, &buffer->buf[cursor], ncopy);
        nread += ncopy;
        to += ncopy;
        
        if(nread < size)
        {
            cursor = 0;
            buffer = container_of(buffer->entry.next, struct ks_circular_buffer_block, entry);
        }
    }
    
    return 0;
}

kboolean ks_circular_buffer_dequeue_array(struct ks_circular_buffer *circular_buffer, void *data, size_t size)
{
    struct ks_circular_buffer_block *buffer;
    size_t nread;
    size_t ncopy;
    unsigned char *to;
    
    if(circular_buffer->usingsize < size)
        return 0;
    
    nread = 0;
    buffer = container_of(circular_buffer->first, struct ks_circular_buffer_block, entry);
    to = data;
    
    while(nread != size)
    {
        ncopy = MIN((KS_CIRCULAR_BUFFER_BLOCK_SIZE - circular_buffer->outpos), (size - nread));
        
        memcpy(to, &buffer->buf[circular_buffer->outpos], ncopy);
        nread += ncopy;
        to += ncopy;
        
        circular_buffer->outpos += ncopy;
        circular_buffer->usingsize -= ncopy;
        
        if(nread < size)
        {
            circular_buffer->outpos = 0;
            buffer = container_of(buffer->entry.next, struct ks_circular_buffer_block, entry);
            list_move_tail(buffer->entry.prev, &circular_buffer->head);
        }
    }
    
    circular_buffer->first = &buffer->entry;
    
    return 0;
}

kboolean ks_circular_buffer_empty(struct ks_circular_buffer *circular_buffer)
{
    if(circular_buffer->usingsize == 0)
        return 1;
    
    if(circular_buffer->tail == circular_buffer->first)
    {
        if(circular_buffer->inpos == circular_buffer->outpos)
            return 1;
    }
    
    return 0;
}

void ks_circular_buffer_reset(struct ks_circular_buffer *circular_buffer)
{
    circular_buffer->inpos = 0;
    circular_buffer->outpos = 0;
    circular_buffer->usingsize = 0;
    circular_buffer->tail = circular_buffer->head.next;
    circular_buffer->first = circular_buffer->head.next;
}

void ks_circular_buffer_addblock(struct ks_circular_buffer *circular_buffer)
{
    struct ks_circular_buffer_block *buffer_block = calloc(1, sizeof(struct ks_circular_buffer_block));
    circular_buffer->totalsize += sizeof(buffer_block->buf);
    list_add_tail(&buffer_block->entry, &circular_buffer->head);
}

///////////////////////////////////////////////////////////////////////////////////////////
static void ks_table_free_node(struct ks_table_slot **slot)
{
    struct ks_table_slot* curr;
    struct ks_table_slot* tmp;
    
    curr = *slot;
    
    while(curr)
    {
        tmp = curr->nextslot;
        free(curr);
        curr = tmp;
    }
    
    *slot = NULL;
}

void ks_table_clear(struct ks_table *table)
{
    uint32_t i;
    
    for(i = 0; i < table->max_slots; i++)
    {
        ks_table_free_node(&table->slots[i]);
        table->slots[i] = NULL;
    }
    
    table->member_count = 0;
}

void INIT_KS_TABLE(struct ks_table *table, int max_slots)
{
	table->member_count = 0;
	table->max_slots = max_slots;
	table->slots = calloc(max_slots, sizeof(struct ks_table_slot *));
}

void ks_table_destroy(struct ks_table *table)
{
    ks_table_clear(table);
    free(table->slots);
    table->slots = NULL;
}

kboolean ks_table_insert(struct ks_table *table, uint64_t id, void* data)
{
    uint32_t index;
    struct ks_table_slot *current, *new;
    
    index = id % table->max_slots;
    
    new = calloc(1, sizeof(struct ks_table_slot));
	new->slotid = id;
	new->data = data;
    new->nextslot = NULL;

    if(table->slots[index] == NULL)
    {
		table->slots[index] = new;
        table->member_count++;
        
        return 1;
    }
    else
    {
        current = table->slots[index];
        
        while(current)
        {
            if(current->slotid == id)
            {
                free(new);
                
                return 0;
            }
            current = current->nextslot;
        }
        
		new->nextslot = table->slots[index];
		table->slots[index] = new;
        table->member_count++;
    }
    
    return 1;
}

kboolean ks_table_remove(struct ks_table *table, uint64_t id)
{
    uint32_t index;
    struct ks_table_slot *current, *prev;
    
    index = id % table->max_slots;
    
    current = table->slots[index];
    prev = NULL;
    
    if(current)
    {
        do
        {
            if(current->slotid == id)
            {
                if(prev)
                {
                    table->member_count--;
                    prev->nextslot = current->nextslot;
                }
                else
                {
                    table->member_count--;
                    table->slots[index] = current->nextslot;
                }
                
                free(current);
                return 1;
            }
            
            prev = current;
            current = current->nextslot;
        }while(current);
    }	
    
    return 0;
}

void *ks_table_find(struct ks_table *table, uint64_t id)
{
    struct ks_table_slot *current;
    uint32_t index;
    
    index = id % table->max_slots;
    current = table->slots[index];
    
    while(current)
    {
        if(current->slotid == id)
        {
            return current->data;
        }
        
        current = current->nextslot;
    }
    
    return NULL;
}

void ks_table_enum(struct ks_table *table, ks_table_callback cb,void *user_arg)
{
    uint32_t i;
    struct ks_table_slot *current, *temp;

	current = NULL;
	temp = NULL;

    for(i = 0; i < table->max_slots; i++)
    {
        current = table->slots[i];

        while(current)
        {
		temp = current->nextslot;
		cb(table, current->slotid, current->data, user_arg);
		current = temp;
        }
    }
}

void INIT_KS_SOCKET_CONTAINER(struct ks_socket_container *container, uv_loop_t *loop, struct ks_socket_callback *callback, int max_connections, int init_socket_count, int max_slots, int init_buffers_count, int init_writereq_count)
{
	int i;
	struct ks_socket_context *context;
	struct ks_buffer *buffer;
	struct ks_writereq *wreq;

	container->callback = callback;
	container->loop = loop;
	container->uniqid_iterators = 1;
	container->init_buffers_count = init_buffers_count;
	container->init_writereq_count = init_writereq_count;
	container->max_connections = max_connections;
	container->init_socket_count = init_socket_count;
	container->current_socket_count = 0;
	container->num_connections = 0;
	
	INIT_LIST_HEAD(&container->active_connections);
	INIT_LIST_HEAD(&container->inactive_connections);
	INIT_LIST_HEAD(&container->buffers);
	INIT_LIST_HEAD(&container->using_buffers);
	INIT_LIST_HEAD(&container->writereq_buffers);
	
	INIT_KS_TABLE(&container->connections, max_slots);
	
	for(i = 0; i < init_socket_count; i++)
	{
		context = callback->socket_context_new(container);
		list_add_tail(&context->entry, &container->inactive_connections);
	}

	for(i = 0; i < init_buffers_count; i++)
	{
		buffer = ks_buffer_create();
		list_add(&buffer->entry, &container->buffers);
	}

	for(i = 0; i < init_writereq_count; i++)
	{
		wreq = calloc(1, sizeof(struct ks_writereq));
		list_add(&wreq->entry, &container->writereq_buffers);
	}
}

kboolean ks_socket_getsockname(const struct ks_socket_context *context, struct ks_netadr *netadr)
{
    int namelen = sizeof(struct sockaddr);
    size_t bufferlen = sizeof(netadr->addr_unix.sun_path);
    
    bzero(netadr, sizeof(struct ks_netadr));
    
    if(context->handle.handle.type == UV_TCP)
    {
        if(uv_tcp_getsockname(&context->handle.tcp, &netadr->addr, &namelen) == 0)
            return 1;
    }
 
    if(context->handle.handle.type == UV_NAMED_PIPE)
    {
        if(uv_pipe_getsockname(&context->handle.pipe, netadr->addr_unix.sun_path, &bufferlen) == 0)
            return 1;
    }
    
    return 0;
}

kboolean ks_socket_getpeername(const struct ks_socket_context *context, struct ks_netadr *netadr)
{
    int namelen = sizeof(struct sockaddr);
    size_t bufferlen = sizeof(netadr->addr_unix.sun_path);
    
    bzero(netadr, sizeof(struct ks_netadr));
    
    if(context->handle.handle.type == UV_TCP)
    {
        if(uv_tcp_getpeername(&context->handle.tcp, &netadr->addr, &namelen) == 0)
            return 1;
    }
    else if(context->handle.handle.type == UV_NAMED_PIPE)
    {
        if(uv_pipe_getpeername(&context->handle.pipe, netadr->addr_unix.sun_path, &bufferlen) == 0)
            return 1;
    }
    
    return 0;
}


struct ks_socket_context* ks_socket_refernece(struct ks_socket_container *container, struct ks_socket_context *context)
{
	if(context == NULL)
	{
		if(list_empty(&container->inactive_connections))
		{
			context = container->callback->socket_context_new(container);
		}
		else
		{
			context = list_first_entry(&container->inactive_connections, struct ks_socket_context, entry);
			list_del(&context->entry);
		}

		context->active = 1;
		context->refcount = 1;
		context->status = KS_SOCKET_STATUS_UNINITIALIZE;
		context->container = container;
		context->uniqid = __sync_add_and_fetch(&container->uniqid_iterators, 1);
		context->handle.handle.data = context;
		context->after_close_disconnected = 0;
		INIT_LIST_HEAD(&context->sendorder);

		list_add_tail(&context->entry, &container->active_connections);
		ks_table_insert(&container->connections, context->uniqid, context);

		if(container->callback->socket_init)
		{
			container->callback->socket_init(container, context);
		}
	}
	else
	{
		__sync_add_and_fetch(&context->refcount, 1);
	}

	return context;
}

void ks_socket_derefernece(struct ks_socket_container *container, struct ks_socket_context *context)
{
	int64_t refcount;

	refcount = __sync_sub_and_fetch(&context->refcount, 1);

	if(refcount <= 0)
	{
		assert(list_empty(&context->sendorder));
		assert(context->status == KS_SOCKET_STATUS_CLOSED || context->status == KS_SOCKET_STATUS_UNINITIALIZE);

		if(container->callback->socket_uninit)
		{
			container->callback->socket_uninit(container, context);
		}
		context->active = 0;
		bzero(&context->handle, sizeof(context->handle));
		context->status = KS_SOCKET_STATUS_UNINITIALIZE;
		ks_table_remove(&container->connections, context->uniqid);
		list_del(&context->entry);
		list_add_tail(&context->entry, &container->inactive_connections);
	}
}

static void ks_socket_handle_closed(uv_handle_t *handle)
{
	struct ks_socket_context *context = handle->data;

	if(context->after_close_disconnected)
	{
		if(context->container->callback->disconnected)
		{
			context->container->callback->disconnected(context->container, context);
		}
		context->container->num_connections --;
	}

	context->status = KS_SOCKET_STATUS_CLOSED;
	ks_socket_derefernece(context->container, context);
}

static void ks_socket_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	struct ks_socket_context *context = handle->data;
	buf->base = context->rdbuf;
	buf->len = sizeof(context->rdbuf);
}

static void ks_connected_cb(uv_connect_t *req, int status)
{
	struct ks_socket_context *context = req->data;

	if(status == 0)
	{
		context->status = KS_SOCKET_STATUS_ESTABLISHED;

		if(context->container->callback->connected)
		{
			context->container->callback->connected(context->container, context);
		}
	}
	else
	{
		if(context->container->callback->connect_failed)
		{
			context->container->callback->connect_failed(context->container, context, status);
		}

		context->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&context->handle.handle, ks_socket_handle_closed); 
	}
}

static void ks_socket_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	struct ks_socket_context *context = stream->data;

	if(nread == UV_EOF)
	{
		if(context->container->callback->disconnected)
		{
			context->container->callback->disconnected(context->container, context);
		}

		context->container->num_connections--;
		context->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return;
	}
	
	if(nread < 0)
	{
		if(context->container->callback->handle_error)
		{
			context->container->callback->handle_error(context->container, context, (int)nread);
		}
		if(context->container->callback->disconnected)
		{
			context->container->callback->disconnected(context->container, context);
		}

		context->container->num_connections--;
		context->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return;
	}

	if(context->container->callback->arrived)
	{
		context->container->callback->arrived(context->container, context, buf->base, nread);
	}
}

static void ks_socket_connection_cb(uv_stream_t *server, int status)
{
	int err;

	struct ks_socket_container *container;
	struct ks_socket_context *context = server->data;
	struct ks_socket_context *newcontext;

	if(status)
	{
		return;
	}

	err = 0;

	container = context->container;

	newcontext = ks_socket_refernece(container, NULL);
	if(newcontext == NULL)
	{
		fprintf(stderr, "out of memory\n");
		return;
	}

	if(server->type == UV_NAMED_PIPE)
	{
		err = uv_pipe_init(container->loop, &newcontext->handle.pipe, 0);
	}

	if(server->type == UV_TCP)
	{
		err = uv_tcp_init(container->loop, &newcontext->handle.tcp);
	}
	
	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, newcontext, err);
		}

		ks_socket_derefernece(container, newcontext);
		return;
	}
	
	newcontext->status = KS_SOCKET_STATUS_INITIALIZED;

	err = uv_accept(server, &newcontext->handle.stream);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, newcontext, err);
		}

		newcontext->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&newcontext->handle.handle, ks_socket_handle_closed);
		return;
	}

	newcontext->status = KS_SOCKET_STATUS_ESTABLISHED;

	if(container->num_connections + 1 > container->max_connections)
	{
		if(container->callback->handle_serverfull)
		{
			container->callback->handle_serverfull(container, newcontext);
		}

		ks_socket_shutdown(newcontext);
		return;
	}

	container->num_connections++;	

	err = uv_read_start(&newcontext->handle.stream, ks_socket_alloc_cb, ks_socket_read_cb);
	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, newcontext, err);
		}

		newcontext->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&newcontext->handle.handle, ks_socket_handle_closed);
		return;
	}

	if(container->callback->connected)
	{
		container->callback->connected(container, newcontext);
	}
}

int ks_socket_addlistener_ipv4(struct ks_socket_container *container, const char *addr, int port)
{
	int err;
	struct sockaddr_in adr;
	struct ks_socket_context *context;

	err = uv_ip4_addr(addr, port, &adr);

	if(err)
	{
		return err;
	}

	context = ks_socket_refernece(container, NULL);
	if(context == NULL)
	{
		fprintf(stderr, "out of memory\n");
		abort();
	}

	err = uv_tcp_init(container->loop, &context->handle.tcp);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}
		ks_socket_derefernece(container, context);
		return err;
	}

	context->status = KS_SOCKET_STATUS_INITIALIZED;

	err = uv_tcp_bind(&context->handle.tcp, (const struct sockaddr *)&adr, 0);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}
		context->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return err;
	}

	err = uv_listen(&context->handle.stream, SOMAXCONN, ks_socket_connection_cb);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}

		context->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return err;
	}

	context->status = KS_SOCKET_STATUS_LISTEN;

	return 0;
}

int ks_socket_addlistener_ipv6(struct ks_socket_container *container, const char *addr, int port)
{
	int err;
	struct sockaddr_in6 adr;
	struct ks_socket_context *context;

	err = uv_ip6_addr(addr, port, &adr);

	if(err)
	{
		return err;
	}

	context = ks_socket_refernece(container, NULL);
	if(context == NULL)
	{
		fprintf(stderr, "out of memory\n");
		abort();
	}

	err = uv_tcp_init(container->loop, &context->handle.tcp);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}
		ks_socket_derefernece(container, context);
		return err;
	}

	context->status = KS_SOCKET_STATUS_INITIALIZED;

	err = uv_tcp_bind(&context->handle.tcp, (const struct sockaddr *)&adr, 0);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}
		context->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return err;
	}

	err = uv_listen(&context->handle.stream, SOMAXCONN, ks_socket_connection_cb);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}
		context->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return err;
	}

	context->status = KS_SOCKET_STATUS_LISTEN;

	return 0;
}

int ks_socket_addlistener_pipe(struct ks_socket_container *container, const char *path)
{
	int err;
	struct ks_socket_context *context;

	context = ks_socket_refernece(container, NULL);
	if(context == NULL)
	{
		fprintf(stderr, "out of memory\n");
		abort();
	}

	err = uv_pipe_init(container->loop, &context->handle.pipe, 0);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}
		ks_socket_derefernece(container, context);
		return err;
	}

	context->status = KS_SOCKET_STATUS_INITIALIZED;

	err = uv_pipe_bind(&context->handle.pipe, path);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}
		context->status = KS_SOCKET_STATUS_CLOSING;

		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return err;
	}

	err = uv_listen(&context->handle.stream, SOMAXCONN, ks_socket_connection_cb);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}
		context->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return err;
	}

	context->status = KS_SOCKET_STATUS_LISTEN;

	return 0;
}

int ks_socket_connect_ipv4(struct ks_socket_container *container, const char *addr, int port)
{
	int err;
	struct sockaddr_in adr;
	struct ks_socket_context *context;

	err = uv_ip4_addr(addr, port, &adr);

	if(err)
	{
		return err;
	}

	context = ks_socket_refernece(container, NULL);
	if(context == NULL)
	{
		fprintf(stderr, "out of memory\n");
		abort();
	}
	
	err = uv_tcp_init(container->loop, &context->handle.tcp);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}
		ks_socket_derefernece(container, context);
		return err;
	}
	context->status = KS_SOCKET_STATUS_INITIALIZED;

	context->connect_req.data = context;

	err = uv_tcp_connect(&context->connect_req, &context->handle.tcp, &adr, ks_connected_cb);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}

		context->status = KS_SOCKET_STATUS_CLOSING;

		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return err;
	}

	context->status = KS_SOCKET_STATUS_CONNECTING;

	return 0;
}

int ks_socket_connect_ipv6(struct ks_socket_container *container, const char *addr, int port)
{
	int err;
	struct sockaddr_in6 adr;
	struct ks_socket_context *context;

	err = uv_ip6_addr(addr, port, &adr);

	if(err)
	{
		return err;
	}

	context = ks_socket_refernece(container, NULL);
	if(context == NULL)
	{
		fprintf(stderr, "out of memory\n");
		abort();
	}
	
	err = uv_tcp_init(container->loop, &context->handle.tcp);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}
		ks_socket_derefernece(container, context);
		return err;
	}
	context->status = KS_SOCKET_STATUS_INITIALIZED;

	context->connect_req.data = context;

	err = uv_tcp_connect(&context->connect_req, &context->handle.tcp, &adr, ks_connected_cb);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}

		context->status = KS_SOCKET_STATUS_CLOSING;

		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return err;
	}

	context->status = KS_SOCKET_STATUS_CONNECTING;

	return 0;
}
int ks_socket_connect_pipe(struct ks_socket_container *container, const char *name)
{
	int err;
	struct ks_socket_context *context;

	context = ks_socket_refernece(container, NULL);
	if(context == NULL)
	{
		fprintf(stderr, "out of memory\n");
		abort();
	}

	err = uv_pipe_init(container->loop, &context->handle.pipe, 0);

	if(err)
	{
		if(container->callback->handle_error)
		{
			container->callback->handle_error(container, context, err);
		}
		ks_socket_derefernece(container, context);
		return err;
	}

	context->status = KS_SOCKET_STATUS_INITIALIZED;
	
	context->connect_req.data = context;

	uv_pipe_connect(&context->connect_req, &context->handle.pipe, name, ks_connected_cb);
	context->status = KS_SOCKET_STATUS_CONNECTING;
	return 0;
}

struct ks_buffer *ks_socket_buffer_refernece(struct ks_socket_container *container, struct ks_buffer *buffer)
{
	if(buffer)
	{
		ks_buffer_addref(buffer);
		return buffer;
	}

	if(list_empty(&container->buffers))
	{
		buffer = ks_buffer_create();
		list_add_tail(&buffer->entry, &container->using_buffers);

		ks_buffer_addref(buffer);
		
		return buffer;
	}

	buffer = list_last_entry(&container->buffers, struct ks_buffer, entry);
	list_del(&buffer->entry);
	list_add_tail(&buffer->entry, &container->using_buffers);
	ks_buffer_addref(buffer);


	return buffer;
}

void ks_socket_buffer_derefernece(struct ks_socket_container *container, struct ks_buffer *buffer)
{
	if(ks_buffer_decref(buffer))
	{
		list_del(&buffer->entry);
		ks_buffer_reset(buffer);
		list_add_tail(&buffer->entry, &container->buffers);
	}
}

static void ks_write_cb(uv_write_t *req, int status)
{
	struct ks_socket_container *container;
	struct ks_writereq *writereq = req->data;
	
	container = writereq->context->container;

	if(writereq->context->container->callback->send_notify)
	{
		writereq->context->container->callback->send_notify(container, writereq->context, writereq->buffer, status);
	}

	list_del(&writereq->entry);
	ks_socket_buffer_derefernece(container, writereq->buffer);
	ks_socket_derefernece(container, writereq->context);

	writereq->buffer = NULL;
	writereq->bufptr.base = NULL;
	writereq->bufptr.len = 0;
	writereq->context = NULL;

	list_add_tail(&writereq->entry, &container->writereq_buffers);
}
int ks_socket_send(struct ks_socket_context *context, struct ks_buffer *buffer)
{
	int err;
	struct ks_writereq *wreq;

	if(context->status == KS_SOCKET_STATUS_ESTABLISHED)
	{
		if(list_empty(&context->container->writereq_buffers))
		{
			wreq = calloc(1, sizeof(struct ks_writereq));
		}
		else
		{
			wreq = list_last_entry(&context->container->writereq_buffers, struct ks_writereq, entry);
			list_del(&wreq->entry);
		}

		ks_socket_refernece(context->container, context);
		ks_socket_buffer_refernece(context->container, buffer);
		list_add(&wreq->entry, &context->sendorder);

		wreq->context = context;
		wreq->buffer = buffer;
		wreq->bufptr.base = ks_buffer_getdata(buffer);
		wreq->bufptr.len = ks_buffer_size(buffer);
		wreq->wrequest.data = wreq;

		err = uv_write(&wreq->wrequest, &context->handle.stream, &wreq->bufptr, 1, ks_write_cb);

		if(err)
		{
			list_del(&wreq->entry);
			ks_socket_buffer_derefernece(context->container, buffer);
			ks_socket_derefernece(context->container, context);

			wreq->buffer = NULL;
			wreq->bufptr.base = NULL;
			wreq->bufptr.len = 0;
			wreq->context = NULL;

			list_add_tail(&wreq->entry, &context->container->writereq_buffers);

			uv_close(&context->handle.handle, ks_socket_handle_closed);
			return err;
		}
		
		return 0;
	}

	return 1;
}

static void ks_shutdown_cb(uv_shutdown_t *req, int status)
{
	struct ks_socket_context *context = req->data;

	context->status = KS_SOCKET_STATUS_SHUTDOWN;

	if(context->container->callback->disconnected)
	{
		context->container->callback->disconnected(context->container, context);
	}

	context->status = KS_SOCKET_STATUS_CLOSING;
	uv_close(&context->handle.handle, ks_socket_handle_closed);
}

int ks_socket_shutdown(struct ks_socket_context *context)
{
	int err;
	if(context->status == KS_SOCKET_STATUS_ESTABLISHED)
	{
		context->status = KS_SOCKET_STATUS_SHUTTINGDOWN;
		context->shutdown_req.data = context;

		err = uv_shutdown(&context->shutdown_req, &context->handle.stream, ks_shutdown_cb);
		
		if(err)
		{
			context->after_close_disconnected = 1;
			context->status = KS_SOCKET_STATUS_CLOSING;
			uv_close(&context->handle.handle, ks_socket_handle_closed);
			return err;
		}

		return 0;
	}
	return 1;
}

int ks_socket_close(struct ks_socket_context *context)
{
	if(context->status == KS_SOCKET_STATUS_ESTABLISHED)
	{
		context->after_close_disconnected = 1;
		context->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return 0;
	}

	if(context->status == KS_SOCKET_STATUS_LISTEN)
	{
		context->status = KS_SOCKET_STATUS_CLOSING;
		uv_close(&context->handle.handle, ks_socket_handle_closed);
		return 0;
	}

	return 1;
}

int ks_socket_stop(struct ks_socket_container *container)
{
	return 1;
}
//=======================================================================================================

struct ks_socket_context *my_socket_context_new(struct ks_socket_container *container)
{
	return calloc(1, sizeof(struct ks_socket_context));
}

void my_socket_context_free(struct ks_socket_container *container, struct ks_socket_context *context)
{
	free(context);
}

void my_socket_init(struct ks_socket_container *container, struct ks_socket_context *context)
{

}

void my_socket_uninit(struct ks_socket_container *container, struct ks_socket_context *context)
{
}

void my_connected(struct ks_socket_container *container, struct ks_socket_context *context)
{
}

void my_disconnected(struct ks_socket_container *container, struct ks_socket_context *context)
{
}

void my_arrived(struct ks_socket_container *container, struct ks_socket_context *context, const char *data, ssize_t nread)
{
/*
	struct ks_buffer *buffer;
	buffer = ks_socket_buffer_refernece(container ,NULL);

	ks_buffer_write(buffer, "hello world", 12);

	ks_socket_send(context, buffer);

	ks_socket_buffer_derefernece(container, buffer);
*/
	ks_socket_close(context);
}

void my_received(struct ks_socket_container *container, struct ks_socket_context *context, struct ks_buffer *buffer)
{
}

void my_handle_error(struct ks_socket_container *container, struct ks_socket_context *context, int err)
{
}

struct ks_socket_callback callback = 
{
	.socket_context_new = my_socket_context_new,
	.socket_context_free = my_socket_context_free,
	.socket_init = my_socket_init,
	.socket_uninit = my_socket_uninit,
	.connected = my_connected,
	.disconnected = my_disconnected,
	.arrived = my_arrived, 
	.received = my_received,
	.handle_error = my_handle_error
};

int main(int argc, char *argv[]) 
{
	struct ks_socket_container socket_container;
	
	INIT_KS_SOCKET_CONTAINER(&socket_container, uv_default_loop(), &callback, 20000, 30000, 65535, 200000, 200000);

	ks_socket_addlistener_ipv4(&socket_container, "0.0.0.0", 27015);

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);

}