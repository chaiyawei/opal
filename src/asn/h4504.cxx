//
// h4504.cxx
//
// Code automatically generated by asnparse.
//

#ifdef P_USE_PRAGMA
#pragma implementation "h4504.h"
#endif

#include <ptlib.h>
#include "asn/h4504.h"

#define new PNEW


#if ! H323_DISABLE_H4504



//
// CallHoldOperation
//

H4504_CallHoldOperation::H4504_CallHoldOperation(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Enumeration(tag, tagClass, 104, FALSE
#ifndef PASN_NOPRINTON
      , "holdNotific=101 "
        "retrieveNotific "
        "remoteHold "
        "remoteRetrieve "
#endif
    )
{
}


H4504_CallHoldOperation & H4504_CallHoldOperation::operator=(unsigned v)
{
  SetValue(v);
  return *this;
}


PObject * H4504_CallHoldOperation::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(H4504_CallHoldOperation::Class()), PInvalidCast);
#endif
  return new H4504_CallHoldOperation(*this);
}


//
// MixedExtension
//

H4504_MixedExtension::H4504_MixedExtension(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Choice(tag, tagClass, 2, FALSE
#ifndef PASN_NOPRINTON
      , "extension "
        "nonStandardData "
#endif
    )
{
}


#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
H4504_MixedExtension::operator H4501_Extension &() const
#else
H4504_MixedExtension::operator H4501_Extension &()
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), H4501_Extension), PInvalidCast);
#endif
  return *(H4501_Extension *)choice;
}


H4504_MixedExtension::operator const H4501_Extension &() const
#endif
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), H4501_Extension), PInvalidCast);
#endif
  return *(H4501_Extension *)choice;
}


#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
H4504_MixedExtension::operator H225_NonStandardParameter &() const
#else
H4504_MixedExtension::operator H225_NonStandardParameter &()
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), H225_NonStandardParameter), PInvalidCast);
#endif
  return *(H225_NonStandardParameter *)choice;
}


H4504_MixedExtension::operator const H225_NonStandardParameter &() const
#endif
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), H225_NonStandardParameter), PInvalidCast);
#endif
  return *(H225_NonStandardParameter *)choice;
}


BOOL H4504_MixedExtension::CreateObject()
{
  switch (tag) {
    case e_extension :
      choice = new H4501_Extension();
      return TRUE;
    case e_nonStandardData :
      choice = new H225_NonStandardParameter();
      return TRUE;
  }

  choice = NULL;
  return FALSE;
}


PObject * H4504_MixedExtension::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(H4504_MixedExtension::Class()), PInvalidCast);
#endif
  return new H4504_MixedExtension(*this);
}


//
// Extension
//

H4504_Extension::H4504_Extension(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Sequence(tag, tagClass, 0, FALSE, 0)
{
}


#ifndef PASN_NOPRINTON
void H4504_Extension::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  strm << setw(indent+14) << "extensionId = " << setprecision(indent) << m_extensionId << '\n';
  strm << setw(indent+11) << "argument = " << setprecision(indent) << m_argument << '\n';
  strm << setw(indent-1) << "}";
}
#endif


PObject::Comparison H4504_Extension::Compare(const PObject & obj) const
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(this, H4504_Extension), PInvalidCast);
#endif
  const H4504_Extension & other = (const H4504_Extension &)obj;

  Comparison result;

  if ((result = m_extensionId.Compare(other.m_extensionId)) != EqualTo)
    return result;
  if ((result = m_argument.Compare(other.m_argument)) != EqualTo)
    return result;

  return PASN_Sequence::Compare(other);
}


PINDEX H4504_Extension::GetDataLength() const
{
  PINDEX length = 0;
  length += m_extensionId.GetObjectLength();
  length += m_argument.GetObjectLength();
  return length;
}


BOOL H4504_Extension::Decode(PASN_Stream & strm)
{
  if (!PreambleDecode(strm))
    return FALSE;

  if (!m_extensionId.Decode(strm))
    return FALSE;
  if (!m_argument.Decode(strm))
    return FALSE;

  return UnknownExtensionsDecode(strm);
}


void H4504_Extension::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);

  m_extensionId.Encode(strm);
  m_argument.Encode(strm);

  UnknownExtensionsEncode(strm);
}


PObject * H4504_Extension::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(H4504_Extension::Class()), PInvalidCast);
#endif
  return new H4504_Extension(*this);
}


//
// ArrayOf_MixedExtension
//

H4504_ArrayOf_MixedExtension::H4504_ArrayOf_MixedExtension(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Array(tag, tagClass)
{
}


PASN_Object * H4504_ArrayOf_MixedExtension::CreateObject() const
{
  return new H4504_MixedExtension;
}


H4504_MixedExtension & H4504_ArrayOf_MixedExtension::operator[](PINDEX i) const
{
  return (H4504_MixedExtension &)array[i];
}


PObject * H4504_ArrayOf_MixedExtension::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(H4504_ArrayOf_MixedExtension::Class()), PInvalidCast);
#endif
  return new H4504_ArrayOf_MixedExtension(*this);
}


//
// HoldNotificArg
//

H4504_HoldNotificArg::H4504_HoldNotificArg(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Sequence(tag, tagClass, 1, TRUE, 0)
{
  m_extensionArg.SetConstraints(PASN_Object::FixedConstraint, 0, 255);
}


#ifndef PASN_NOPRINTON
void H4504_HoldNotificArg::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  if (HasOptionalField(e_extensionArg))
    strm << setw(indent+15) << "extensionArg = " << setprecision(indent) << m_extensionArg << '\n';
  strm << setw(indent-1) << "}";
}
#endif


PObject::Comparison H4504_HoldNotificArg::Compare(const PObject & obj) const
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(this, H4504_HoldNotificArg), PInvalidCast);
#endif
  const H4504_HoldNotificArg & other = (const H4504_HoldNotificArg &)obj;

  Comparison result;

  if ((result = m_extensionArg.Compare(other.m_extensionArg)) != EqualTo)
    return result;

  return PASN_Sequence::Compare(other);
}


PINDEX H4504_HoldNotificArg::GetDataLength() const
{
  PINDEX length = 0;
  if (HasOptionalField(e_extensionArg))
    length += m_extensionArg.GetObjectLength();
  return length;
}


BOOL H4504_HoldNotificArg::Decode(PASN_Stream & strm)
{
  if (!PreambleDecode(strm))
    return FALSE;

  if (HasOptionalField(e_extensionArg) && !m_extensionArg.Decode(strm))
    return FALSE;

  return UnknownExtensionsDecode(strm);
}


void H4504_HoldNotificArg::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);

  if (HasOptionalField(e_extensionArg))
    m_extensionArg.Encode(strm);

  UnknownExtensionsEncode(strm);
}


PObject * H4504_HoldNotificArg::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(H4504_HoldNotificArg::Class()), PInvalidCast);
#endif
  return new H4504_HoldNotificArg(*this);
}


//
// RetrieveNotificArg
//

H4504_RetrieveNotificArg::H4504_RetrieveNotificArg(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Sequence(tag, tagClass, 1, TRUE, 0)
{
  m_extensionArg.SetConstraints(PASN_Object::FixedConstraint, 0, 255);
}


#ifndef PASN_NOPRINTON
void H4504_RetrieveNotificArg::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  if (HasOptionalField(e_extensionArg))
    strm << setw(indent+15) << "extensionArg = " << setprecision(indent) << m_extensionArg << '\n';
  strm << setw(indent-1) << "}";
}
#endif


PObject::Comparison H4504_RetrieveNotificArg::Compare(const PObject & obj) const
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(this, H4504_RetrieveNotificArg), PInvalidCast);
#endif
  const H4504_RetrieveNotificArg & other = (const H4504_RetrieveNotificArg &)obj;

  Comparison result;

  if ((result = m_extensionArg.Compare(other.m_extensionArg)) != EqualTo)
    return result;

  return PASN_Sequence::Compare(other);
}


PINDEX H4504_RetrieveNotificArg::GetDataLength() const
{
  PINDEX length = 0;
  if (HasOptionalField(e_extensionArg))
    length += m_extensionArg.GetObjectLength();
  return length;
}


BOOL H4504_RetrieveNotificArg::Decode(PASN_Stream & strm)
{
  if (!PreambleDecode(strm))
    return FALSE;

  if (HasOptionalField(e_extensionArg) && !m_extensionArg.Decode(strm))
    return FALSE;

  return UnknownExtensionsDecode(strm);
}


void H4504_RetrieveNotificArg::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);

  if (HasOptionalField(e_extensionArg))
    m_extensionArg.Encode(strm);

  UnknownExtensionsEncode(strm);
}


PObject * H4504_RetrieveNotificArg::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(H4504_RetrieveNotificArg::Class()), PInvalidCast);
#endif
  return new H4504_RetrieveNotificArg(*this);
}


//
// RemoteHoldArg
//

H4504_RemoteHoldArg::H4504_RemoteHoldArg(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Sequence(tag, tagClass, 1, TRUE, 0)
{
  m_extensionArg.SetConstraints(PASN_Object::FixedConstraint, 0, 255);
}


#ifndef PASN_NOPRINTON
void H4504_RemoteHoldArg::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  if (HasOptionalField(e_extensionArg))
    strm << setw(indent+15) << "extensionArg = " << setprecision(indent) << m_extensionArg << '\n';
  strm << setw(indent-1) << "}";
}
#endif


PObject::Comparison H4504_RemoteHoldArg::Compare(const PObject & obj) const
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(this, H4504_RemoteHoldArg), PInvalidCast);
#endif
  const H4504_RemoteHoldArg & other = (const H4504_RemoteHoldArg &)obj;

  Comparison result;

  if ((result = m_extensionArg.Compare(other.m_extensionArg)) != EqualTo)
    return result;

  return PASN_Sequence::Compare(other);
}


PINDEX H4504_RemoteHoldArg::GetDataLength() const
{
  PINDEX length = 0;
  if (HasOptionalField(e_extensionArg))
    length += m_extensionArg.GetObjectLength();
  return length;
}


BOOL H4504_RemoteHoldArg::Decode(PASN_Stream & strm)
{
  if (!PreambleDecode(strm))
    return FALSE;

  if (HasOptionalField(e_extensionArg) && !m_extensionArg.Decode(strm))
    return FALSE;

  return UnknownExtensionsDecode(strm);
}


void H4504_RemoteHoldArg::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);

  if (HasOptionalField(e_extensionArg))
    m_extensionArg.Encode(strm);

  UnknownExtensionsEncode(strm);
}


PObject * H4504_RemoteHoldArg::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(H4504_RemoteHoldArg::Class()), PInvalidCast);
#endif
  return new H4504_RemoteHoldArg(*this);
}


//
// RemoteHoldRes
//

H4504_RemoteHoldRes::H4504_RemoteHoldRes(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Sequence(tag, tagClass, 1, TRUE, 0)
{
  m_extensionRes.SetConstraints(PASN_Object::FixedConstraint, 0, 255);
}


#ifndef PASN_NOPRINTON
void H4504_RemoteHoldRes::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  if (HasOptionalField(e_extensionRes))
    strm << setw(indent+15) << "extensionRes = " << setprecision(indent) << m_extensionRes << '\n';
  strm << setw(indent-1) << "}";
}
#endif


PObject::Comparison H4504_RemoteHoldRes::Compare(const PObject & obj) const
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(this, H4504_RemoteHoldRes), PInvalidCast);
#endif
  const H4504_RemoteHoldRes & other = (const H4504_RemoteHoldRes &)obj;

  Comparison result;

  if ((result = m_extensionRes.Compare(other.m_extensionRes)) != EqualTo)
    return result;

  return PASN_Sequence::Compare(other);
}


PINDEX H4504_RemoteHoldRes::GetDataLength() const
{
  PINDEX length = 0;
  if (HasOptionalField(e_extensionRes))
    length += m_extensionRes.GetObjectLength();
  return length;
}


BOOL H4504_RemoteHoldRes::Decode(PASN_Stream & strm)
{
  if (!PreambleDecode(strm))
    return FALSE;

  if (HasOptionalField(e_extensionRes) && !m_extensionRes.Decode(strm))
    return FALSE;

  return UnknownExtensionsDecode(strm);
}


void H4504_RemoteHoldRes::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);

  if (HasOptionalField(e_extensionRes))
    m_extensionRes.Encode(strm);

  UnknownExtensionsEncode(strm);
}


PObject * H4504_RemoteHoldRes::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(H4504_RemoteHoldRes::Class()), PInvalidCast);
#endif
  return new H4504_RemoteHoldRes(*this);
}


//
// RemoteRetrieveArg
//

H4504_RemoteRetrieveArg::H4504_RemoteRetrieveArg(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Sequence(tag, tagClass, 1, TRUE, 0)
{
  m_extensionArg.SetConstraints(PASN_Object::FixedConstraint, 0, 255);
}


#ifndef PASN_NOPRINTON
void H4504_RemoteRetrieveArg::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  if (HasOptionalField(e_extensionArg))
    strm << setw(indent+15) << "extensionArg = " << setprecision(indent) << m_extensionArg << '\n';
  strm << setw(indent-1) << "}";
}
#endif


PObject::Comparison H4504_RemoteRetrieveArg::Compare(const PObject & obj) const
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(this, H4504_RemoteRetrieveArg), PInvalidCast);
#endif
  const H4504_RemoteRetrieveArg & other = (const H4504_RemoteRetrieveArg &)obj;

  Comparison result;

  if ((result = m_extensionArg.Compare(other.m_extensionArg)) != EqualTo)
    return result;

  return PASN_Sequence::Compare(other);
}


PINDEX H4504_RemoteRetrieveArg::GetDataLength() const
{
  PINDEX length = 0;
  if (HasOptionalField(e_extensionArg))
    length += m_extensionArg.GetObjectLength();
  return length;
}


BOOL H4504_RemoteRetrieveArg::Decode(PASN_Stream & strm)
{
  if (!PreambleDecode(strm))
    return FALSE;

  if (HasOptionalField(e_extensionArg) && !m_extensionArg.Decode(strm))
    return FALSE;

  return UnknownExtensionsDecode(strm);
}


void H4504_RemoteRetrieveArg::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);

  if (HasOptionalField(e_extensionArg))
    m_extensionArg.Encode(strm);

  UnknownExtensionsEncode(strm);
}


PObject * H4504_RemoteRetrieveArg::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(H4504_RemoteRetrieveArg::Class()), PInvalidCast);
#endif
  return new H4504_RemoteRetrieveArg(*this);
}


//
// RemoteRetrieveRes
//

H4504_RemoteRetrieveRes::H4504_RemoteRetrieveRes(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Sequence(tag, tagClass, 1, TRUE, 0)
{
  m_extensionRes.SetConstraints(PASN_Object::FixedConstraint, 0, 255);
}


#ifndef PASN_NOPRINTON
void H4504_RemoteRetrieveRes::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  if (HasOptionalField(e_extensionRes))
    strm << setw(indent+15) << "extensionRes = " << setprecision(indent) << m_extensionRes << '\n';
  strm << setw(indent-1) << "}";
}
#endif


PObject::Comparison H4504_RemoteRetrieveRes::Compare(const PObject & obj) const
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(this, H4504_RemoteRetrieveRes), PInvalidCast);
#endif
  const H4504_RemoteRetrieveRes & other = (const H4504_RemoteRetrieveRes &)obj;

  Comparison result;

  if ((result = m_extensionRes.Compare(other.m_extensionRes)) != EqualTo)
    return result;

  return PASN_Sequence::Compare(other);
}


PINDEX H4504_RemoteRetrieveRes::GetDataLength() const
{
  PINDEX length = 0;
  if (HasOptionalField(e_extensionRes))
    length += m_extensionRes.GetObjectLength();
  return length;
}


BOOL H4504_RemoteRetrieveRes::Decode(PASN_Stream & strm)
{
  if (!PreambleDecode(strm))
    return FALSE;

  if (HasOptionalField(e_extensionRes) && !m_extensionRes.Decode(strm))
    return FALSE;

  return UnknownExtensionsDecode(strm);
}


void H4504_RemoteRetrieveRes::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);

  if (HasOptionalField(e_extensionRes))
    m_extensionRes.Encode(strm);

  UnknownExtensionsEncode(strm);
}


PObject * H4504_RemoteRetrieveRes::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(H4504_RemoteRetrieveRes::Class()), PInvalidCast);
#endif
  return new H4504_RemoteRetrieveRes(*this);
}


#endif // if ! H323_DISABLE_H4504


// End of h4504.cxx
