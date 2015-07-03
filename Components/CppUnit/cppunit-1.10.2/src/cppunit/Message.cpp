#include <cppunit/Message.h>
#include <stdexcept>


CPPUNIT_NS_BEGIN


Message::Message()
{
}


Message::Message( const std::string &shortDescription )
    : m_shortDescription( shortDescription )
{
}

Message::Message( const std::string &shortDescription,
                  const std::string &detail1 )
    : m_shortDescription( shortDescription )
{
  addDetail( detail1 );
}

Message::Message( const std::string &shortDescription,
                  const std::string &detail1,
                  const std::string &detail2 )
    : m_shortDescription( shortDescription )
{
  addDetail( detail1, detail2 );
}


Message::Message( const std::string &shortDescription,
                  const std::string &detail1,
                  const std::string &detail2,
                  const std::string &detail3 )
    : m_shortDescription( shortDescription )
{
  addDetail( detail1, detail2, detail3 );
}


const std::string &
Message::shortDescription() const
{
  return m_shortDescription;
}


int 
Message::detailCount() const
{
  return m_details.size();
}

/* make:  Error code 133, while making 'Release\Message.o' */
std::string 
Message::detailAt( int index ) const
{
#if defined (CPPUNIT_WITHOUT_CPP_STREAM)
  if (!( index < 0  ||  index >= detailCount() ))
    return m_details[ index ];
  else
    return ("");
#else
  if ( index < 0  ||  index >= detailCount() )
    throw std::invalid_argument( "Message::detailAt() : invalid index" );

  return m_details[ index ];
#endif
}

std::string 
Message::details() const
{
  std::string details;
  for ( Details::const_iterator it = m_details.begin(); it != m_details.end(); ++it )
  {
    details += "- ";
    details += *it;
    details += '\n';
  }
  return details;
}


void 
Message::clearDetails()
{
  m_details.clear();
}


void 
Message::addDetail( const std::string &detail )
{
  m_details.push_back( detail );
}


void 
Message::addDetail( const std::string &detail1,
                    const std::string &detail2 )
{
  addDetail( detail1 );
  addDetail( detail2 );
}


void 
Message::addDetail( const std::string &detail1,
                    const std::string &detail2,
                    const std::string &detail3 )
{
  addDetail( detail1, detail2 );
  addDetail( detail3 );
}


void 
Message::addDetail( const Message &message )
{
  m_details.insert( m_details.end(), 
                    message.m_details.begin(), 
                    message.m_details.end() );
}


void 
Message::setShortDescription( const std::string &shortDescription )
{
  m_shortDescription = shortDescription;
}


bool 
Message::operator ==( const Message &other ) const
{
  return m_shortDescription == other.m_shortDescription  &&
         m_details == other.m_details;
}


bool 
Message::operator !=( const Message &other ) const
{
  return !( *this == other );
}

CPPUNIT_NS_END

