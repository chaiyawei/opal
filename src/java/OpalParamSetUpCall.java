/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.36
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.opalvoip;

public class OpalParamSetUpCall {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected OpalParamSetUpCall(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(OpalParamSetUpCall obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      exampleJNI.delete_OpalParamSetUpCall(swigCPtr);
    }
    swigCPtr = 0;
  }

  public void setM_partyA(String value) {
    exampleJNI.OpalParamSetUpCall_m_partyA_set(swigCPtr, this, value);
  }

  public String getM_partyA() {
    return exampleJNI.OpalParamSetUpCall_m_partyA_get(swigCPtr, this);
  }

  public void setM_partyB(String value) {
    exampleJNI.OpalParamSetUpCall_m_partyB_set(swigCPtr, this, value);
  }

  public String getM_partyB() {
    return exampleJNI.OpalParamSetUpCall_m_partyB_get(swigCPtr, this);
  }

  public void setM_callToken(String value) {
    exampleJNI.OpalParamSetUpCall_m_callToken_set(swigCPtr, this, value);
  }

  public String getM_callToken() {
    return exampleJNI.OpalParamSetUpCall_m_callToken_get(swigCPtr, this);
  }

  public void setM_alertingType(String value) {
    exampleJNI.OpalParamSetUpCall_m_alertingType_set(swigCPtr, this, value);
  }

  public String getM_alertingType() {
    return exampleJNI.OpalParamSetUpCall_m_alertingType_get(swigCPtr, this);
  }

  public void setM_protocolCallId(String value) {
    exampleJNI.OpalParamSetUpCall_m_protocolCallId_set(swigCPtr, this, value);
  }

  public String getM_protocolCallId() {
    return exampleJNI.OpalParamSetUpCall_m_protocolCallId_get(swigCPtr, this);
  }

  public OpalParamSetUpCall() {
    this(exampleJNI.new_OpalParamSetUpCall(), true);
  }

}
