// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RAMSESREFERENCE_RLOGIC_SERIALIZATION_H_
#define FLATBUFFERS_GENERATED_RAMSESREFERENCE_RLOGIC_SERIALIZATION_H_

#include "flatbuffers/flatbuffers.h"

namespace rlogic_serialization {

struct RamsesReference;
struct RamsesReferenceBuilder;

inline const flatbuffers::TypeTable *RamsesReferenceTypeTable();

struct RamsesReference FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef RamsesReferenceBuilder Builder;
  struct Traits;
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return RamsesReferenceTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_OBJECTID = 4,
    VT_OBJECTTYPE = 6
  };
  uint64_t objectId() const {
    return GetField<uint64_t>(VT_OBJECTID, 0);
  }
  uint32_t objectType() const {
    return GetField<uint32_t>(VT_OBJECTTYPE, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint64_t>(verifier, VT_OBJECTID) &&
           VerifyField<uint32_t>(verifier, VT_OBJECTTYPE) &&
           verifier.EndTable();
  }
};

struct RamsesReferenceBuilder {
  typedef RamsesReference Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_objectId(uint64_t objectId) {
    fbb_.AddElement<uint64_t>(RamsesReference::VT_OBJECTID, objectId, 0);
  }
  void add_objectType(uint32_t objectType) {
    fbb_.AddElement<uint32_t>(RamsesReference::VT_OBJECTTYPE, objectType, 0);
  }
  explicit RamsesReferenceBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RamsesReferenceBuilder &operator=(const RamsesReferenceBuilder &);
  flatbuffers::Offset<RamsesReference> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<RamsesReference>(end);
    return o;
  }
};

inline flatbuffers::Offset<RamsesReference> CreateRamsesReference(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t objectId = 0,
    uint32_t objectType = 0) {
  RamsesReferenceBuilder builder_(_fbb);
  builder_.add_objectId(objectId);
  builder_.add_objectType(objectType);
  return builder_.Finish();
}

struct RamsesReference::Traits {
  using type = RamsesReference;
  static auto constexpr Create = CreateRamsesReference;
};

inline const flatbuffers::TypeTable *RamsesReferenceTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_ULONG, 0, -1 },
    { flatbuffers::ET_UINT, 0, -1 }
  };
  static const char * const names[] = {
    "objectId",
    "objectType"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 2, type_codes, nullptr, nullptr, names
  };
  return &tt;
}

}  // namespace rlogic_serialization

#endif  // FLATBUFFERS_GENERATED_RAMSESREFERENCE_RLOGIC_SERIALIZATION_H_
