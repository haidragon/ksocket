// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: helloworld.proto

#ifndef PROTOBUF_helloworld_2eproto__INCLUDED
#define PROTOBUF_helloworld_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3004000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
class HelloWorldRequest;
class HelloWorldRequestDefaultTypeInternal;
extern HelloWorldRequestDefaultTypeInternal _HelloWorldRequest_default_instance_;
class HelloWorldResponse;
class HelloWorldResponseDefaultTypeInternal;
extern HelloWorldResponseDefaultTypeInternal _HelloWorldResponse_default_instance_;

namespace protobuf_helloworld_2eproto {
// Internal implementation detail -- do not call these.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[];
  static const ::google::protobuf::uint32 offsets[];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static void InitDefaultsImpl();
};
void AddDescriptors();
void InitDefaults();
}  // namespace protobuf_helloworld_2eproto

// ===================================================================

class HelloWorldRequest : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:HelloWorldRequest) */ {
 public:
  HelloWorldRequest();
  virtual ~HelloWorldRequest();

  HelloWorldRequest(const HelloWorldRequest& from);

  inline HelloWorldRequest& operator=(const HelloWorldRequest& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  HelloWorldRequest(HelloWorldRequest&& from) noexcept
    : HelloWorldRequest() {
    *this = ::std::move(from);
  }

  inline HelloWorldRequest& operator=(HelloWorldRequest&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const HelloWorldRequest& default_instance();

  static inline const HelloWorldRequest* internal_default_instance() {
    return reinterpret_cast<const HelloWorldRequest*>(
               &_HelloWorldRequest_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(HelloWorldRequest* other);
  friend void swap(HelloWorldRequest& a, HelloWorldRequest& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline HelloWorldRequest* New() const PROTOBUF_FINAL { return New(NULL); }

  HelloWorldRequest* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const HelloWorldRequest& from);
  void MergeFrom(const HelloWorldRequest& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(HelloWorldRequest* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string text = 1;
  void clear_text();
  static const int kTextFieldNumber = 1;
  const ::std::string& text() const;
  void set_text(const ::std::string& value);
  #if LANG_CXX11
  void set_text(::std::string&& value);
  #endif
  void set_text(const char* value);
  void set_text(const char* value, size_t size);
  ::std::string* mutable_text();
  ::std::string* release_text();
  void set_allocated_text(::std::string* text);

  // @@protoc_insertion_point(class_scope:HelloWorldRequest)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr text_;
  mutable int _cached_size_;
  friend struct protobuf_helloworld_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class HelloWorldResponse : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:HelloWorldResponse) */ {
 public:
  HelloWorldResponse();
  virtual ~HelloWorldResponse();

  HelloWorldResponse(const HelloWorldResponse& from);

  inline HelloWorldResponse& operator=(const HelloWorldResponse& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  HelloWorldResponse(HelloWorldResponse&& from) noexcept
    : HelloWorldResponse() {
    *this = ::std::move(from);
  }

  inline HelloWorldResponse& operator=(HelloWorldResponse&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const HelloWorldResponse& default_instance();

  static inline const HelloWorldResponse* internal_default_instance() {
    return reinterpret_cast<const HelloWorldResponse*>(
               &_HelloWorldResponse_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(HelloWorldResponse* other);
  friend void swap(HelloWorldResponse& a, HelloWorldResponse& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline HelloWorldResponse* New() const PROTOBUF_FINAL { return New(NULL); }

  HelloWorldResponse* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const HelloWorldResponse& from);
  void MergeFrom(const HelloWorldResponse& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(HelloWorldResponse* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string text = 1;
  void clear_text();
  static const int kTextFieldNumber = 1;
  const ::std::string& text() const;
  void set_text(const ::std::string& value);
  #if LANG_CXX11
  void set_text(::std::string&& value);
  #endif
  void set_text(const char* value);
  void set_text(const char* value, size_t size);
  ::std::string* mutable_text();
  ::std::string* release_text();
  void set_allocated_text(::std::string* text);

  // @@protoc_insertion_point(class_scope:HelloWorldResponse)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr text_;
  mutable int _cached_size_;
  friend struct protobuf_helloworld_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// HelloWorldRequest

// string text = 1;
inline void HelloWorldRequest::clear_text() {
  text_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& HelloWorldRequest::text() const {
  // @@protoc_insertion_point(field_get:HelloWorldRequest.text)
  return text_.GetNoArena();
}
inline void HelloWorldRequest::set_text(const ::std::string& value) {
  
  text_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:HelloWorldRequest.text)
}
#if LANG_CXX11
inline void HelloWorldRequest::set_text(::std::string&& value) {
  
  text_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:HelloWorldRequest.text)
}
#endif
inline void HelloWorldRequest::set_text(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  text_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:HelloWorldRequest.text)
}
inline void HelloWorldRequest::set_text(const char* value, size_t size) {
  
  text_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:HelloWorldRequest.text)
}
inline ::std::string* HelloWorldRequest::mutable_text() {
  
  // @@protoc_insertion_point(field_mutable:HelloWorldRequest.text)
  return text_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* HelloWorldRequest::release_text() {
  // @@protoc_insertion_point(field_release:HelloWorldRequest.text)
  
  return text_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void HelloWorldRequest::set_allocated_text(::std::string* text) {
  if (text != NULL) {
    
  } else {
    
  }
  text_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), text);
  // @@protoc_insertion_point(field_set_allocated:HelloWorldRequest.text)
}

// -------------------------------------------------------------------

// HelloWorldResponse

// string text = 1;
inline void HelloWorldResponse::clear_text() {
  text_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& HelloWorldResponse::text() const {
  // @@protoc_insertion_point(field_get:HelloWorldResponse.text)
  return text_.GetNoArena();
}
inline void HelloWorldResponse::set_text(const ::std::string& value) {
  
  text_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:HelloWorldResponse.text)
}
#if LANG_CXX11
inline void HelloWorldResponse::set_text(::std::string&& value) {
  
  text_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:HelloWorldResponse.text)
}
#endif
inline void HelloWorldResponse::set_text(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  text_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:HelloWorldResponse.text)
}
inline void HelloWorldResponse::set_text(const char* value, size_t size) {
  
  text_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:HelloWorldResponse.text)
}
inline ::std::string* HelloWorldResponse::mutable_text() {
  
  // @@protoc_insertion_point(field_mutable:HelloWorldResponse.text)
  return text_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* HelloWorldResponse::release_text() {
  // @@protoc_insertion_point(field_release:HelloWorldResponse.text)
  
  return text_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void HelloWorldResponse::set_allocated_text(::std::string* text) {
  if (text != NULL) {
    
  } else {
    
  }
  text_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), text);
  // @@protoc_insertion_point(field_set_allocated:HelloWorldResponse.text)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_helloworld_2eproto__INCLUDED