/*
 * h460_std19.cxx
 *
 * H.460.19 H225 NAT Traversal class.
 *
 * Copyright (c) 2008 ISVO (Asia) Pte. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the General Public License (the  "GNU License"), in which case the
 * provisions of GNU License are applicable instead of those
 * above. If you wish to allow use of your version of this file only
 * under the terms of the GNU License and not to allow others to use
 * your version of this file under the MPL, indicate your decision by
 * deleting the provisions above and replace them with the notice and
 * other provisions required by the GNU License. If you do not delete
 * the provisions above, a recipient may use your version of this file
 * under either the MPL or the GNU License."
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 *
 * The Initial Developer of the Original Code is ISVO (Asia) Pte. Ltd.
 *
 * Portions of this code were written with the assisance of funding from
 * triple-IT. http://www.triple-it.nl.
 *
 * Contributor(s): Many thanks to Simon Horne.
 *                 Robert Jongbloed (robertj@voxlucida.com.au).
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>

#define P_FORCE_STATIC_PLUGIN 1

#include <h460/h460_std19.h>

#if OPAL_H460_NAT

#include <h323/h323ep.h>
#include <h323/h323pdu.h>
#include <h323/gkclient.h>
#include <h460/h46019.h>
#include <ptclib/random.h>
#include <ptclib/cypher.h>


PCREATE_NAT_PLUGIN(H46019);

#ifdef ANDROID
extern void android_sha1( void*, const PString &);
#endif


///////////////////////////////////////////////////////
// H.460.19
//
// Must Declare for Factory Loader.
H460_FEATURE(Std19, "H.460.19");

H460_FeatureStd19::H460_FeatureStd19()
  : H460_Feature(19)
  , m_natMethod(NULL)
  , m_disabledByH46024(false)
{
  PTRACE(6,"Std19\tInstance Created");

  AddParameter(1);  // add multiplex parameter
}


bool H460_FeatureStd19::Initialise(H323EndPoint & ep, H323Connection * con)
{
  if (!H460_Feature::Initialise(ep, con))
    return false;

  m_natMethod = ep.GetNatMethods().GetMethodByName(PNatMethod_H46019::MethodName());
  if (m_natMethod == NULL) {
    PTRACE(1, "Std19\tDisabled as no NAT method");
    return false;
  }

  if (!m_natMethod->IsAvailable(PIPSocket::GetDefaultIpAny())) {
    PTRACE(3, "Std19\tDisabled as NAT method deactivated");
    return false;
  }

  return true;
}


bool H460_FeatureStd19::OnSendingOLCGenericInformation(unsigned sessionID, H245_ArrayOf_GenericParameter & param, bool isAck)
{
  if (m_connection == NULL)
    return false;

  OpalRTPSession * session = dynamic_cast<OpalRTPSession *>(m_connection->GetMediaSession(sessionID));
  if (session == NULL) {
    PTRACE(4,"H46019\tSession " << sessionID << " not found or not RTP.");
    return false;
  }

  H46019UDPSocket * rtp = dynamic_cast<H46019UDPSocket *>(&session->GetDataSocket());
  if (rtp == NULL) {
    PTRACE(3, "H46019\tSession " << sessionID << " RTP socket not generated by H.460.19.");
    return false;
  }

  if (rtp->GetPingPayload() == 0)
    rtp->SetPingPayLoad(127);
  unsigned payload = rtp->GetPingPayload();

  if (rtp->GetTTL() == 0)
    rtp->SetTTL(0);
  unsigned ttl = rtp->GetTTL();

  if (isAck) {
    rtp->Activate();  // Start the RTP Channel if not already started
    H46019UDPSocket * rtcp = dynamic_cast<H46019UDPSocket *>(&session->GetControlSocket());
    if (rtcp != NULL)
      rtcp->Activate();  // Start the RTCP Channel if not already started
  }

  bool h46019msg = false;
  H46019_TraversalParameters params;
  if (/*!isAck ||*/ payload > 0) {
    params.IncludeOptionalField(H46019_TraversalParameters::e_keepAlivePayloadType);
    PASN_Integer & p = params.m_keepAlivePayloadType;
    p = payload;
    h46019msg = true;
  }
  if (/*isAck &&*/ ttl > 0) {
    params.IncludeOptionalField(H46019_TraversalParameters::e_keepAliveInterval);
    H225_TimeToLive & a = params.m_keepAliveInterval;
    a = ttl;
    h46019msg = true;
  }

  if (h46019msg) {
    PTRACE(5,"H46019\tTraversal Parameters:\n" << params);
    param.SetSize(1);

    H245_ParameterIdentifier & id = param[0].m_parameterIdentifier;
    id.SetTag(H245_ParameterIdentifier::e_standard);
    ((PASN_Integer &)id).SetValue(1);

    H245_ParameterValue & val = param[0].m_parameterValue;
    val.SetTag(H245_ParameterValue::e_octetString);
    ((PASN_OctetString &)val).EncodeSubType(params);
  }

  return true;
}


void H460_FeatureStd19::OnReceiveOLCGenericInformation(unsigned sessionID, const H245_ArrayOf_GenericParameter & param)
{
  if (m_connection == NULL)
    return;

  H245_GenericParameter & val = param[0];
  if (val.m_parameterValue.GetTag() != H245_ParameterValue::e_octetString)
    return;

  PASN_OctetString & raw = val.m_parameterValue;
  PPER_Stream pdu(raw);
  H46019_TraversalParameters params;
  if (!params.Decode(pdu)) {
    PTRACE(2,"H46019\tError decoding Traversal Parameters!");
    return;
  }

  PTRACE(4,"H46019\tTraversal Parameters:\n" << params);

  H323TransportAddress RTPaddress;
  H323TransportAddress RTCPaddress;
  if (params.HasOptionalField(H46019_TraversalParameters::e_keepAliveChannel)) {
    H245_TransportAddress & k = params.m_keepAliveChannel;
    RTPaddress = H323TransportAddress(k);
    PIPSocket::Address add; WORD port;
    RTPaddress.GetIpAndPort(add,port);
    RTCPaddress = H323TransportAddress(add,port+1);  // Compute the RTCP Address
  }

  unsigned payload = 0;
  if (params.HasOptionalField(H46019_TraversalParameters::e_keepAlivePayloadType)) {
    PASN_Integer & p = params.m_keepAlivePayloadType;
    payload = p;
  }

  unsigned ttl = 0;
  if (params.HasOptionalField(H46019_TraversalParameters::e_keepAliveInterval)) {
    H225_TimeToLive & a = params.m_keepAliveInterval;
    ttl = a;
  }

  unsigned muxId = 0;
  if (params.HasOptionalField(H46019_TraversalParameters::e_multiplexID)) {
    muxId = params.m_multiplexID;
  }

  OpalRTPSession * session = dynamic_cast<OpalRTPSession *>(m_connection->GetMediaSession(sessionID));
  if (session == NULL) {
    PTRACE(4,"H46019\tSession " << sessionID << " not found or not RTP.");
    return;
  }

  H46019UDPSocket * rtp = dynamic_cast<H46019UDPSocket *>(&session->GetDataSocket());
  if (rtp != NULL)
    rtp->Activate(RTPaddress,payload,ttl, muxId);
  else
    PTRACE(4,"H46019\tSession " << sessionID << " RTP socket not generated by H.460.19.");

  H46019UDPSocket * rtcp = dynamic_cast<H46019UDPSocket *>(&session->GetControlSocket());
  if (rtcp != NULL)
    rtcp->Activate(RTCPaddress,payload,ttl, muxId);
  else
    PTRACE(4,"H46019\tSession " << sessionID << " RTCP socket not generated by H.460.19.");
}


/////////////////////////////////////////////////////////////////////////////

PNatMethod_H46019::PNatMethod_H46019(unsigned priority)
  : PNatMethod(priority)
{
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Default to inactive until we get it all working
  m_active = false; 
}


const char * PNatMethod_H46019::MethodName()
{
  return PPlugin_PNatMethod_H46019::ServiceName();
}


PCaselessString PNatMethod_H46019::GetName() const
{
  return MethodName();
}


PString PNatMethod_H46019::GetServer() const
{
  return PString::Empty();
}


PNatMethod::NatTypes PNatMethod_H46019::InternalGetNatType(bool, const PTimeInterval &)
{
  return PortRestrictedNat; // Assume worst possible NAT type that can do media at all
}


PBoolean PNatMethod_H46019::CreateSocketPair(PUDPSocket * & socket1,
                                             PUDPSocket * & socket2,
                                             const PIPSocket::Address & binding,
                                             void * userData)
{

  PTRACE(2,"H46019\tCreateSocketPair");

  if (m_pairedPortInfo.basePort == 0 || m_pairedPortInfo.basePort > m_pairedPortInfo.maxPort)
  {
    PTRACE(1, "H46019\tInvalid local UDP port range "
      << m_pairedPortInfo.currentPort << '-' << m_pairedPortInfo.maxPort);
    return FALSE;
  }

  OpalRTPSession * session = (OpalRTPSession *)userData;
  H323Connection & connection = dynamic_cast<H323Connection &>(session->GetConnection());
  unsigned sessionID = session->GetSessionID();

  socket1 = new H46019UDPSocket(connection, sessionID, true);    /// Data 
  socket2 = new H46019UDPSocket(connection, sessionID, false);   /// Control

  /// Make sure we have sequential ports
  while ((!OpenSocket(*socket1, m_pairedPortInfo,binding)) ||
    (!OpenSocket(*socket2, m_pairedPortInfo,binding)) ||
    (socket2->GetPort() != socket1->GetPort() + 1 || 
    socket1->GetPort() & 1)  // socket 1 should be even
    )
  {
    PTRACE(5,"H46019\tAttempted Sockets: " << socket1->GetPort() << "," << socket2->GetPort() );
    if (socket1->GetPort() & 1) {
      // oops, somehow incremented one port but not the other....
      PWaitAndSignal mutex(m_pairedPortInfo.mutex);
      PTRACE(5,"H46019\tAdding one to currentPort: " << m_pairedPortInfo.currentPort );
      m_pairedPortInfo.currentPort++;
    }
    delete socket1;
    delete socket2;
    socket1 = new H46019UDPSocket(connection, sessionID, true);    /// Data 
    socket2 = new H46019UDPSocket(connection, sessionID, false);   /// Control
  }

  PTRACE(5, "H46019\tUDP ports "
    << socket1->GetPort() << '-' << socket2->GetPort());

  PTRACE(3, "H46019\tSetConnectionSockets sessionId: " << session->GetSessionID());
  return TRUE;
}


PBoolean PNatMethod_H46019::OpenSocket(PUDPSocket & socket,
                                       PortInfo & portInfo,
                                       const PIPSocket::Address & binding) const
{
  PWaitAndSignal mutex(portInfo.mutex);

  WORD startPort = portInfo.currentPort;

  do {
    portInfo.currentPort++;
    if (portInfo.currentPort > portInfo.maxPort)
      portInfo.currentPort = portInfo.basePort;

    if (socket.Listen(binding,1, portInfo.currentPort)) {
      socket.SetReadTimeout(500);
      return true;
    }

  } while (portInfo.currentPort != startPort);

  PTRACE(2, "H46019\tFailed to bind to local UDP port in range "
    << portInfo.currentPort << '-' << portInfo.maxPort);
  return false;
}


bool PNatMethod_H46019::IsAvailable(const PIPSocket::Address & binding)
{ 
  return PNatMethod::IsAvailable(binding);
}


/////////////////////////////////////////////////////////////////////////////////////////////

H46019UDPSocket::H46019UDPSocket(H323Connection & connection, unsigned sessionID, bool rtp)
  : m_connection(connection)
  , m_sessionID(sessionID)
  , m_CUI(PRandom::Number(sessionID*100,sessionID*100+99)) // Some random number bases on the session id (for H.460.24A)
  , keepport(0)
  , keeppayload(0)
  , keepTTL(0)
  , keepseqno(0)
  , keepStartTime(NULL)
  , m_locAddr(PIPSocket::GetInvalidAddress())
  , m_locPort(0)
  , m_remAddr(PIPSocket::GetInvalidAddress())
  , m_remPort(0)
  , m_detAddr(PIPSocket::GetInvalidAddress())
  , m_detPort(0)
  , m_pendAddr(PIPSocket::GetInvalidAddress())
  , m_pendPort(0)
  , m_probes(0)
  , m_SSRC(PRandom::Number())
  , m_altAddr(PIPSocket::GetInvalidAddress())
  , m_altPort(0)
  , m_h46024b(false)
  , m_rtpSocket(rtp)
{
  SetProbeState(e_notRequired);
  PTRACE(4, "H46024A\tGenerated CUI: session=" << m_sessionID << " value=" << m_CUI);
}


H46019UDPSocket::~H46019UDPSocket()
{
  //PTRACE(5, "H46019\tI'm going away: " << (int)this);
  Keep.Stop();
  delete keepStartTime;

  m_Probe.Stop();
}


void H46019UDPSocket::Allocate(const H323TransportAddress & keepalive, unsigned _payload, unsigned _ttl, unsigned _muxId)
{
  PIPSocket::Address ip;
  WORD port = 0;
  keepalive.GetIpAndPort(ip,port);
  if (ip.IsValid() && !ip.IsLoopback() && port > 0) {
    keepip = ip;
    keepport = port;
  }

  if (_payload > 0)
    keeppayload = _payload;

  if (_ttl > 0)
    keepTTL = _ttl;

  if (_muxId > 0)
    muxId = _muxId;

  PTRACE(4,"H46019UDP\tSetting " << keepip << ":" << keepport << " ping " << keepTTL << " secs. MuxId: " << muxId);
}


void H46019UDPSocket::Activate()
{
  InitialiseKeepAlive();
}


void H46019UDPSocket::Activate(const H323TransportAddress & keepalive, unsigned _payload, unsigned _ttl, unsigned _muxId)
{
  Allocate(keepalive,_payload,_ttl,_muxId);
  InitialiseKeepAlive();
}


void H46019UDPSocket::InitialiseKeepAlive() 
{
  PWaitAndSignal m(PingMutex);

  if (Keep.IsRunning()) {
    PTRACE(6,"H46019UDP\t" << (m_rtpSocket ? "RTP" : "RTCP") << " ping already running.");
    return;
  }

  if (keepTTL > 0 && keepip.IsValid() && !keepip.IsLoopback()) {
    keepseqno = 100;  // Some arbitory number
    keepStartTime = new PTime();

    PTRACE(4,"H46019UDP\tStart " << (m_rtpSocket ? "RTP" : "RTCP") << " pinging " 
      << keepip << ":" << keepport << " every " << keepTTL << " secs.");

    m_rtpSocket ? SendRTPPing(keepip,keepport) : SendRTCPPing();

    Keep.SetNotifier(PCREATE_NOTIFIER(Ping));
    Keep.RunContinuous(keepTTL * 1000); 
  } else {
    PTRACE(2,"H46019UDP\t"  << (m_rtpSocket ? "RTP" : "RTCP") << " PING NOT Ready " 
      << keepip << ":" << keepport << " - " << keepTTL << " secs.");

  }
}


void H46019UDPSocket::Ping(PTimer &, P_INT_PTR)
{ 
  m_rtpSocket ? SendRTPPing(keepip,keepport) : SendRTCPPing();
}


void H46019UDPSocket::SendRTPPing(const PIPSocket::Address & ip, const WORD & port) {

  RTP_DataFrame rtp;

  rtp.SetSequenceNumber(keepseqno);

  rtp.SetPayloadType((RTP_DataFrame::PayloadTypes)keeppayload);
  rtp.SetPayloadSize(0);

  // determining correct timestamp
  PTime currentTime = PTime();
  PTimeInterval timePassed = currentTime - *keepStartTime;
  rtp.SetTimestamp((DWORD)timePassed.GetMilliSeconds() * 8);

  rtp.SetMarker(TRUE);

  if (!Internal_WriteTo(rtp.GetPointer(), rtp.GetHeaderSize()+rtp.GetPayloadSize(), ip, port)) {
    switch (GetErrorCode(LastWriteError)) {
      case PChannel::Unavailable :
        PTRACE(2, "H46019UDP\t" << ip << ":" << port << " not ready.");
        break;

      default:
        PTRACE(1, "H46019UDP\t" << ip << ":" << port 
          << ", Write error on port ("
          << GetErrorNumber(PChannel::LastWriteError) << "): "
          << GetErrorText(PChannel::LastWriteError));
    }
  } else {
    PTRACE(6, "H46019UDP\tRTP KeepAlive sent: " << ip << ":" << port << " seq: " << keepseqno);    
    keepseqno++;
  }
}


void H46019UDPSocket::SendRTCPPing() 
{
  RTP_ControlFrame report;
  report.SetPayloadType(RTP_ControlFrame::e_SenderReport);
  report.SetPayloadSize(sizeof(RTP_ControlFrame::SenderReport));

  if (SendRTCPFrame(report, keepip, keepport)) {
    PTRACE(6, "H46019UDP\tRTCP KeepAlive sent: " << keepip << ":" << keepport);    
  }
}


PBoolean H46019UDPSocket::SendRTCPFrame(RTP_ControlFrame & report, const PIPSocket::Address & ip, WORD port) {

  if (!Internal_WriteTo(report.GetPointer(),report.GetSize(), ip, port)) {
    switch (GetErrorCode(LastWriteError)) {
      case PChannel::Unavailable :
        PTRACE(2, "H46019UDP\t" << ip << ":" << port << " not ready.");
        break;

      default:
        PTRACE(1, "H46019UDP\t" << ip << ":" << port 
          << ", Write error on port ("
          << GetErrorNumber(PChannel::LastWriteError) << "): "
          << GetErrorText(PChannel::LastWriteError));
      }
      return false;
  } 
  return true;
}


PBoolean H46019UDPSocket::GetLocalAddress(PIPSocket::Address & addr, WORD & port) 
{
  if (PUDPSocket::GetLocalAddress(addr, port)) {
    m_locAddr = addr;
    m_locPort = port;
    return true;
  }
  return false;
}


unsigned H46019UDPSocket::GetPingPayload()
{
  return keeppayload;
}


void H46019UDPSocket::SetPingPayLoad(unsigned val)
{
  keeppayload = val;
}


unsigned H46019UDPSocket::GetTTL()
{
  return keepTTL;
}


void H46019UDPSocket::SetTTL(unsigned val)
{
  keepTTL = val;
}


void H46019UDPSocket::SetProbeState(probe_state newstate)
{
  PWaitAndSignal m(probeMutex);

  m_state = newstate;
}


int H46019UDPSocket::GetProbeState() const
{
  PWaitAndSignal m(probeMutex);

  return m_state;
}


void H46019UDPSocket::SetAlternateAddresses(const H323TransportAddress & address, const PString & cui)
{
  address.GetIpAndPort(m_altAddr,m_altPort);

  PTRACE(6,"H46024A\ts: " << m_sessionID << (m_rtpSocket ? " RTP " : " RTCP ")  
    << "Remote Alt: " << m_altAddr << ":" << m_altPort << " CUI: " << cui);

  if (!m_rtpSocket) {
    m_CUIrem = cui;
    if (GetProbeState() < e_idle) {
      SetProbeState(e_idle);
      StartProbe();
      // We Already have a direct connection but we are waiting on the CUI for the reply
    } else if (GetProbeState() == e_verify_receiver) 
      ProbeReceived(false,m_pendAddr,m_pendPort);
  }
}


void H46019UDPSocket::GetAlternateAddresses(H323TransportAddress & address, PString & cui)
{

  address = H323TransportAddress(m_locAddr,m_locPort);

  if (!m_rtpSocket)
    cui = m_CUI;
  else
    cui = PString();

  if (GetProbeState() < e_idle)
    SetProbeState(e_initialising);

  PTRACE(6,"H46024A\ts:" << m_sessionID << (m_rtpSocket ? " RTP " : " RTCP ") << " Alt:" << address << " CUI " << cui);

}


PBoolean H46019UDPSocket::IsAlternateAddress(const Address & address,WORD port)
{
  return ((address == m_detAddr) && (port == m_detPort));
}


void H46019UDPSocket::StartProbe()
{

  PTRACE(4,"H46024A\ts: " << m_sessionID << " Starting direct connection probe.");

  SetProbeState(e_probing);
  m_probes = 0;
  m_Probe.SetNotifier(PCREATE_NOTIFIER(Probe));
  m_Probe.RunContinuous(100); 
}


void H46019UDPSocket::BuildProbe(RTP_ControlFrame & report, bool probing)
{
  report.SetPayloadType(RTP_ControlFrame::e_ApplDefined);
  report.SetCount((probing ? 0 : 1));  // SubType Probe

  report.SetPayloadSize(sizeof(probe_packet));

  probe_packet data;
  data.SSRC = m_SSRC;
  data.Length = sizeof(probe_packet);
  PString id = "24.1";
  PBYTEArray bytes(id,id.GetLength(), false);
  memcpy(&data.name[0], bytes, 4);

#if OPAL_PTLIB_SSL
  PMessageDigest::Result bin_digest;
  PMessageDigestSHA1::Encode(m_connection.GetCallIdentifier().AsString() + m_CUIrem, bin_digest);
  memcpy(&data.cui[0], bin_digest.GetPointer(), bin_digest.GetSize());
#else
  //android_sha1(&data.cui[0],m_CallId.AsString() + m_CUIrem);
#endif

  memcpy(report.GetPayloadPtr(),&data,sizeof(probe_packet));

}


void H46019UDPSocket::Probe(PTimer &, P_INT_PTR)
{ 
  m_probes++;

  if (m_probes >= 5) {
    m_Probe.Stop();
    return;
  }

  if (GetProbeState() != e_probing)
    return;

  RTP_ControlFrame report;
  report.SetSize(4+sizeof(probe_packet));
  BuildProbe(report, true);

  if (!PUDPSocket::WriteTo(report.GetPointer(),report.GetSize(), m_altAddr, m_altPort)) {
    switch (GetErrorCode(LastWriteError)) {
      case PChannel::Unavailable :
        PTRACE(2, "H46024A\t" << m_altAddr << ":" << m_altPort << " not ready.");
        break;

      default:
        PTRACE(1, "H46024A\t" << m_altAddr << ":" << m_altPort 
          << ", Write error on port ("
          << GetErrorNumber(PChannel::LastWriteError) << "): "
          << GetErrorText(PChannel::LastWriteError));
      }
  } else {
    PTRACE(6, "H46024A\ts" << m_sessionID <<" RTCP Probe sent: " << m_altAddr << ":" << m_altPort);    
  }
}


void H46019UDPSocket::ProbeReceived(bool probe, const PIPSocket::Address & addr, WORD & port)
{
  if (probe) {
  }
  else {
    RTP_ControlFrame reply;
    reply.SetSize(4+sizeof(probe_packet));
    BuildProbe(reply, false);
    if (SendRTCPFrame(reply,addr,port)) {
      PTRACE(4, "H46024A\tRTCP Reply packet sent: " << addr << ":" << port);    
    }
  }
}


void H46019UDPSocket::H46024Adirect(bool starter)
{
  if (starter) {  // We start the direct channel 
    m_detAddr = m_altAddr;  m_detPort = m_altPort;
    PTRACE(4, "H46024A\ts:" << m_sessionID << (m_rtpSocket ? " RTP " : " RTCP ")  
      << "Switching to " << m_detAddr << ":" << m_detPort);
    SetProbeState(e_direct);
  } else         // We wait for the remote to start channel
    SetProbeState(e_wait);

  Keep.Stop();  // Stop the keepAlive Packets
}


PBoolean H46019UDPSocket::ReadFrom(void * buf, PINDEX len, Address & addr, WORD & port)
{
  bool probe = false; bool success = false;
  RTP_ControlFrame frame(2048); 

  while (PUDPSocket::ReadFrom(buf, len, addr, port)) {

    /// Set the detected routed remote address (on first packet received)
    if (m_remAddr.IsAny()) {   
      m_remAddr = addr; 
      m_remPort = port;
    }
    if (m_h46024b && addr == m_altAddr && port == m_altPort) {
      PTRACE(4, "H46024B\ts:" << m_sessionID << (m_rtpSocket ? " RTP " : " RTCP ")  
        << "Switching to " << addr << ":" << port << " from " << m_remAddr << ":" << m_remPort);
      m_detAddr = addr;  m_detPort = port;
      SetProbeState(e_direct);
      Keep.Stop();  // Stop the keepAlive Packets
      m_h46024b = false;
      continue;
    }
    /// Check the probe state
    switch (GetProbeState()) {
    case e_initialising:                        // RTCP only
    case e_idle:                                // RTCP only
    case e_probing:                                // RTCP only
    case e_verify_receiver:                        // RTCP only
      frame.SetSize(len);
      memcpy(frame.GetPointer(),buf,len);
      if (ReceivedProbePacket(frame,probe,success)) {
        if (success)
          ProbeReceived(probe,addr,port);
        else {
          m_pendAddr = addr; m_pendPort = port;
        }
        continue;  // don't forward on probe packets.
      }
      break;
    case e_wait:
      if (addr == keepip) {// We got a keepalive ping...
        Keep.Stop();  // Stop the keepAlive Packets
      } else if ((addr == m_altAddr) && (port == m_altPort)) {
        PTRACE(4, "H46024A\ts:" << m_sessionID << (m_rtpSocket ? " RTP " : " RTCP ")  << "Already sending direct!");
        m_detAddr = addr;  m_detPort = port;
        SetProbeState(e_direct);
      } else if ((addr == m_pendAddr) && (port == m_pendPort)) {
        PTRACE(4, "H46024A\ts:" << m_sessionID << (m_rtpSocket ? " RTP " : " RTCP ")  
          << "Switching to Direct " << addr << ":" << port);
        m_detAddr = addr;  m_detPort = port;
        SetProbeState(e_direct);
      } else if ((addr != m_remAddr) || (port != m_remPort)) {
        PTRACE(4, "H46024A\ts:" << m_sessionID << (m_rtpSocket ? " RTP " : " RTCP ")  
          << "Switching to " << addr << ":" << port << " from " << m_remAddr << ":" << m_remPort);
        m_detAddr = addr;  m_detPort = port;
        SetProbeState(e_direct);
      } 
      break;
    case e_direct:    
    default:
      break;
    }
    return true;
  } 
  return false; 
}


PBoolean H46019UDPSocket::Internal_WriteTo(const void * bytes, PINDEX len, const Address & addr, WORD port)
{
	if (GetProbeState() == e_direct)
		return PUDPSocket::WriteTo(bytes,len, m_detAddr, m_detPort);
	else
		return PUDPSocket::WriteTo(bytes,len, addr, port);
}


PBoolean H46019UDPSocket::Internal_WriteTo(const Slice* slices, size_t nSlices, const Address & addr, WORD port)
{
  if (GetProbeState() == e_direct)
    return PUDPSocket::WriteTo(slices, nSlices, m_detAddr, m_detPort);
  else
    return PUDPSocket::WriteTo(slices, nSlices, addr, port);
}


PBoolean H46019UDPSocket::WriteTo(const void * buf, PINDEX len, const Address & addr, WORD port) {

  // If a muxId was given in traversal parameters, then we need to tac it on
  // as the 1st four bytes of rtp data.

  Slice slices[2];
  size_t nSlices=0;
  unsigned char muxBuf[4];
  if (muxId > 0) {
    memcpy(muxBuf,&muxId,4);
    slices[nSlices++] = Slice(muxBuf,4);
  }
  slices[nSlices++] = Slice(const_cast<void*>(buf),len);
  return Internal_WriteTo (slices,nSlices,addr, port );
}


PBoolean H46019UDPSocket::ReceivedProbePacket(const RTP_ControlFrame & frame, bool & probe, bool & success)
{
  success = false;

  //Inspect the probe packet
  if (frame.GetPayloadType() == RTP_ControlFrame::e_ApplDefined) {

    int cstate = GetProbeState();
    if (cstate == e_notRequired) {
      PTRACE(6, "H46024A\ts:" << m_sessionID <<" received RTCP probe packet. LOGIC ERROR!");
      return true;  
    }

    if (cstate > e_probing) {
      PTRACE(6, "H46024A\ts:" << m_sessionID <<" received RTCP probe packet. IGNORING! Already authenticated.");
      return true;  
    }

    probe = (frame.GetCount() > 0);
    PTRACE(4, "H46024A\ts:" << m_sessionID <<" RTCP Probe " << (probe ? "Reply" : "Request") << " received.");    

    BYTE * data = frame.GetPayloadPtr();
    PBYTEArray bytes(20);
    memcpy(bytes.GetPointer(),data+12, 20);
#if OPAL_PTLIB_SSL
    PMessageDigest::Result bin_digest;
    PMessageDigestSHA1::Encode(m_connection.GetCallIdentifier().AsString() + m_CUI, bin_digest);
    PBYTEArray val(bin_digest.GetPointer(),bin_digest.GetSize());
#else
    PBYTEArray val(20);
    //android_sha1(val.GetPointer(), m_CallId.AsString() + m_CUI);
#endif

    if (bytes == val) {
      if (probe)  // We have a reply
        SetProbeState(e_verify_sender);
      else 
        SetProbeState(e_verify_receiver);

      m_Probe.Stop();
      PTRACE(4, "H46024A\ts" << m_sessionID <<" RTCP Probe " << (probe ? "Reply" : "Request") << " verified.");
      if (!m_CUIrem.IsEmpty())
        success = true;
      else {
        PTRACE(4, "H46024A\ts" << m_sessionID <<" Remote not ready.");
      }
    } else {
      PTRACE(4, "H46024A\ts" << m_sessionID <<" RTCP Probe " << (probe ? "Reply" : "Request") << " verify FAILURE");    
    }
    return true;
  }
  else 
    return false;
}


void H46019UDPSocket::H46024Bdirect(const H323TransportAddress & address)
{
  address.GetIpAndPort(m_altAddr,m_altPort);
  PTRACE(6,"H46024b\ts: " << m_sessionID << " RTP Remote Alt: " << m_altAddr << ":" << m_altPort);

  m_h46024b = true;

  // Sending an empty RTP frame to the alternate address
  // will add a mapping to the router to receive RTP from
  // the remote
  SendRTPPing(m_altAddr, m_altPort);
}


#endif  // OPAL_H460_NAT