#include <iostream>
#include "ns3/mp-tcp-typedefs.h"
#include "ns3/simulator.h"
#include "ns3/log.h"


NS_LOG_COMPONENT_DEFINE("MpTcpMapping");

namespace ns3
{

MpTcpMapping::MpTcpMapping() :
  m_dataSequenceNumber(0),
  m_subflowSequenceNumber(0),
  m_dataLevelLength(0)
{
  NS_LOG_FUNCTION(this);
}

void
MpTcpMapping::SetMappingSize(uint16_t const& length)
{
  NS_LOG_LOGIC(this << length);
  m_dataLevelLength = length;
}

bool
MpTcpMapping::TranslateSSNToDSN(const SequenceNumber32& ssn,SequenceNumber32& dsn) const
{
  if(IsSSNInRange(ssn))
  {
//      dsn =
//    NS_FATAL_ERROR("TODO");
  // TODO check for seq wrapping ? PAWS
    dsn = SequenceNumber32(ssn - HeadSSN()) + HeadDSN();
    return true;
  }

  return false;
}


std::ostream&
operator<<(std::ostream& os, const MpTcpMapping& mapping)
{
  //
  os << "DSN [" << mapping.HeadDSN() << "-" << mapping.TailDSN ()
  //of size [" << mapping.GetLength() <<"] from DSN [" << mapping.HeadDSN()
    << "] mapped to SSN [" <<  mapping.HeadSSN() << "-" <<  mapping.TailSSN() << "]";
  return os;
}

void
MpTcpMapping::SetDSN(SequenceNumber32 const& dsn)
{
  NS_LOG_LOGIC(this << dsn);
  m_dataSequenceNumber = dsn;
}


void
MpTcpMapping::MapToSSN( SequenceNumber32 const& seq)
{
  NS_LOG_LOGIC(this << " mapping to ssn ["<< seq << "]");
  m_subflowSequenceNumber = seq;
}

bool
MpTcpMapping::Intersect(const MpTcpMapping& mapping) const
{
  //!
  return( IsSSNInRange( mapping.HeadSSN()) || IsSSNInRange( mapping.TailSSN())
         || IsDSNInRange( mapping.HeadDSN()) || IsDSNInRange( mapping.TailDSN()) );
}

bool
MpTcpMapping::operator==( const MpTcpMapping& mapping) const
{
  //!
  return (
    GetLength() == mapping.GetLength()
    && HeadDSN() == mapping.HeadDSN()
//    && GetLength()  == GetLength()
    );
}

SequenceNumber32
MpTcpMapping::HeadDSN() const
{
  return m_dataSequenceNumber;
}

// TODO rename into GetMappedSSN Head ?
SequenceNumber32
MpTcpMapping::HeadSSN() const
{
  return m_subflowSequenceNumber;
}

uint16_t
MpTcpMapping::GetLength() const
{
  NS_LOG_FUNCTION(this);
  return m_dataLevelLength;
}


SequenceNumber32
MpTcpMapping::TailDSN(void) const
{
  return(HeadDSN()+GetLength()-1);
}

SequenceNumber32
MpTcpMapping::TailSSN(void) const
{
//  NS_LOG_UNCOND("TailSSN called");
//  GetLength();
//  NS_LOG_UNCOND("GetLength called");
//  HeadSSN();
//  NS_LOG_UNCOND("HeadSSN called");
  return(HeadSSN()+GetLength()-1);
}

bool
MpTcpMapping::operator<(MpTcpMapping const& m) const
{

  return (HeadDSN() < m.HeadDSN());
}


bool
MpTcpMapping::IsSSNInRange(SequenceNumber32 const& ssn) const
{

  return ( HeadSSN() <= ssn && TailSSN() >= ssn );
}

bool
MpTcpMapping::IsDSNInRange(SequenceNumber32 const& dsn) const
{

  return ( HeadDSN() <= dsn && TailDSN() >= dsn );
}


//SequenceNumber32 subflowSeqNb
void
MpTcpMapping::Configure( SequenceNumber32  dataSeqNb, uint16_t mappingSize)
//  m_dataSeqNumber(dataSeqNb),
//  m_size(mappingSize)
{
  NS_LOG_LOGIC(this << "dsn ["<< dataSeqNb << "], mappingSize [" << mappingSize << "]");
  m_dataSequenceNumber = dataSeqNb;
  m_dataLevelLength = mappingSize;
}

///////////////////////////////////////////////////////////
///// MpTcpMappingContainer
/////
MpTcpMappingContainer::MpTcpMappingContainer(void) :
  m_txBuffer(0),
  m_rxBuffer(0)
{
  NS_LOG_LOGIC(this);
}

MpTcpMappingContainer::~MpTcpMappingContainer(void)
{
  NS_LOG_LOGIC(this);
}

void
MpTcpMappingContainer::Dump()
{
  NS_LOG_UNCOND("\n\n==== Dumping list of mappings ====");
  for( MappingList::iterator it = m_mappings.begin(); it != m_mappings.end(); it++ )
  {

    NS_LOG_UNCOND( *it );

  }
  NS_LOG_UNCOND("==== End of dump ====\n");
}
//
//void
//MpTcpMappingContainer::DiscardMappingsUpToSSN(const SequenceNumber32& ssn)
//{
//  //!
//  for( MappingList::iterator it = m_mappings.begin(); it != m_mappings.end(); it++ )
//  {
//
//    NS_LOG_UNCOND( *it );
//
//  }
//
//}


  //!
int
MpTcpMappingContainer::AddMappingEnforceSSN(const MpTcpMapping& mapping)
{
  for( MappingList::iterator it = m_mappings.begin(); it != m_mappings.end(); it++ )
  {

    if(it->Intersect(mapping)   )
    {
      NS_LOG_WARN("Mappings " << mapping << " intersect with " << *it );
      return -1;
    }

  }
  m_mappings.insert(mapping);
  return 0;
}


int
MpTcpMappingContainer::AddMappingLooseSSN(MpTcpMapping& mapping)
{
  NS_ASSERT_MSG(m_txBuffer,"m_txBuffer not set");
  // TODO look for duplicatas
  mapping.MapToSSN( FirstUnmappedSSN() );

  return AddMappingEnforceSSN(mapping);

}

//SequenceNumber32
//MpTcpMappingContainer::FirstMappedSSN(void) const
//{
//  //!
//}

SequenceNumber32
MpTcpMappingContainer::FirstUnmappedSSN(void) const
{
  NS_ASSERT(m_txBuffer);
  if(m_mappings.empty())
  {
//    if(m_rxBuffer){
//      return m_rxBuffer->TailSequence();
//    }
//    else {
//      NS_ASSERT
      // associate to first byte in buffer. This should never happen ?
      return m_txBuffer->HeadSequence();
//    }
  }
//  else {
    // they are sorted
  NS_LOG_INFO("\n\n====================\n\n");
//  NS_LOG_INFO( *m_mappings.end() );
  return m_mappings.rbegin()->TailSSN() + 1;
//  }
}

bool
MpTcpMappingContainer::TranslateSSNtoDSN(const SequenceNumber32& ssn,SequenceNumber32 &dsn)
{
  // first find if a mapping exists
  MpTcpMapping mapping;
  if(!GetMappingForSSN( ssn, mapping) )
  {
    //!
    return false;
  }

  return mapping.TranslateSSNToDSN(ssn,dsn);
}

void
MpTcpMappingContainer::DiscardMappingsUpToSSN(const SequenceNumber32& ssn)
{
  NS_LOG_INFO("Discarding mappings up to " << ssn);
  MappingList& l = m_mappings;
  // TODO use reverse iterator and then clear from first found to the begin
  for( MappingList::iterator it = l.begin(); it != l.end(); it++ )
  {
    //HeadDSN
    if( it->TailSSN() < ssn )
    {
      //it =
//      NS_ASSERT( );
      // TODO check mapping transfer was completed on this subflow
//      if( m_txBuffer.HeadSequence() <  )
//      {
//
//      }
      l.erase(it);
    }
  }
}

#if 0
void
MpTcpMappingContainer::DiscardMappingsUpToDSN(const SequenceNumber32& dsn)
{
  NS_LOG_INFO("Discarding mappings up to " << dsn);
  MappingList& l = m_mappings;
  for( MappingList::iterator it = l.begin(); it != l.end(); it++ )
  {
    //HeadDSN
    if( it->TailDSN() < dsn )
    {
      //it =
//      NS_ASSERT( );
      // TODO check mapping transfer was completed on this subflow
//      if( m_txBuffer.HeadSequence() <  )
//      {
//
//      }
      l.erase(it);
    }
  }
}

#endif

bool
MpTcpMappingContainer::GetMappingForSSN(const SequenceNumber32& ssn, MpTcpMapping& mapping)
{
  MappingList& l = m_mappings;
  for( MappingList::const_iterator it = l.begin(); it != l.end(); it++ )
  {
    // check seq nb is within the DSN range
    if (
      it->IsSSNInRange( ssn )
//    (subflowSeqNb >= it->HeadSSN() ) &&
//      (subflowSeqNb < it->HeadSSN() + it->GetLength())
    )
    {
      mapping = *it;
      return true;
    }
  }

  return false;
}



} // namespace ns3
