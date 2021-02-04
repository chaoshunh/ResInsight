/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "RicMswSegment.h"

#include <memory>
#include <vector>

class RicMswBranch
{
public:
    RicMswBranch( double initialMD = 0.0, double initialTVD = 0.0 );

    void addSegment( std::shared_ptr<RicMswSegment> location );
    void sortSegments();

    double initialMD() const;
    double initialTVD() const;

    const std::vector<std::shared_ptr<RicMswSegment>>& segments() const;
    std::vector<std::shared_ptr<RicMswSegment>>&       segments();

    void addChildBranch( std::unique_ptr<RicMswBranch> branch );

    std::vector<const RicMswBranch*> childBranches() const;
    std::vector<RicMswBranch*>       childBranches();

private:
    double m_initialMD;
    double m_initialTVD;

    std::vector<std::shared_ptr<RicMswSegment>> m_segments;
    std::vector<std::unique_ptr<RicMswBranch>>  m_childBranches;
};
