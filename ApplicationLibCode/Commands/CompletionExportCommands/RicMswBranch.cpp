#include "RicMswBranch.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswBranch::RicMswBranch( double initialMD, double initialTVD )
    : m_initialMD( initialMD )
    , m_initialTVD( initialTVD )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::addSegment( std::shared_ptr<RicMswSegment> location )
{
    m_segments.push_back( location );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::sortSegments()
{
    std::sort( m_segments.begin(),
               m_segments.end(),
               []( std::shared_ptr<RicMswSegment> lhs, std::shared_ptr<RicMswSegment> rhs ) { return *lhs < *rhs; } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswBranch::initialMD() const
{
    return m_initialMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswBranch::initialTVD() const
{
    return m_initialTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::shared_ptr<RicMswSegment>>& RicMswBranch::segments() const
{
    return m_segments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<RicMswSegment>>& RicMswBranch::segments()
{
    return m_segments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswBranch::addChildBranch( std::unique_ptr<RicMswBranch> branch )
{
    m_childBranches.push_back( std::move( branch ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RicMswBranch*> RicMswBranch::childBranches() const
{
    std::vector<const RicMswBranch*> branches;
    for ( const auto& branch : m_childBranches )
    {
        branches.push_back( branch.get() );
    }
    return branches;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicMswBranch*> RicMswBranch::childBranches()
{
    std::vector<RicMswBranch*> branches;
    for ( const auto& branch : m_childBranches )
    {
        branches.push_back( branch.get() );
    }
    return branches;
}
