/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimStimPlanModelTemplate.h"

#include "RiaColorTables.h"
#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"
#include "RiaStimPlanModelDefines.h"

#include "Riu3DMainWindowTools.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "Rim3dView.h"
#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimElasticProperties.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFaciesProperties.h"
#include "RimNonNetLayers.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanModelPlot.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"
#include "cvfMatrix4.h"
#include "cvfPlane.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimStimPlanModelTemplate, "StimPlanModelTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelTemplate::RimStimPlanModelTemplate()
    : changed( this )
{
    CAF_PDM_InitScriptableObject( "StimPlanModelTemplate", "", "", "" );

    CAF_PDM_InitScriptableField( &m_id, "Id", -1, "ID", "", "", "" );
    m_id.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableField( &m_defaultPorosity,
                                 "DefaultPorosity",
                                 RiaDefines::defaultPorosity(),
                                 "Default Porosity",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_defaultPermeability,
                                 "DefaultPermeability",
                                 RiaDefines::defaultPermeability(),
                                 "Default Permeability",
                                 "",
                                 "",
                                 "" );

    // Stress unit: bar
    // Stress gradient unit: bar/m
    // Depth is meter
    double defaultStressGradient = 0.238;
    double defaultStressDepth    = computeDefaultStressDepth();
    double defaultStress         = defaultStressDepth * defaultStressGradient;

    CAF_PDM_InitScriptableField( &m_verticalStress, "VerticalStress", defaultStress, "Vertical Stress", "", "", "" );
    m_verticalStress.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
    CAF_PDM_InitScriptableField( &m_verticalStressGradient,
                                 "VerticalStressGradient",
                                 defaultStressGradient,
                                 "Vertical Stress Gradient",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_stressDepth, "StressDepth", defaultStressDepth, "Stress Depth", "", "", "" );
    m_stressDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_referenceTemperature, "ReferenceTemperature", 70.0, "Temperature [C]", "", "", "" );
    CAF_PDM_InitScriptableField( &m_referenceTemperatureGradient,
                                 "ReferenceTemperatureGradient",
                                 0.025,
                                 "Temperature Gradient [C/m]",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_referenceTemperatureDepth,
                                 "ReferenceTemperatureDepth",
                                 2500.0,
                                 "Temperature Depth [m]",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableField( &m_overburdenHeight, "OverburdenHeight", 50.0, "Overburden Height", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_overburdenFormation, "OverburdenFormation", "Overburden Formation", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_overburdenFacies, "OverburdenFacies", "Overburden Facies", "", "", "" );
    CAF_PDM_InitScriptableField( &m_overburdenPorosity, "OverburdenPorosity", 0.0, "Overburden Porosity", "", "", "" );
    CAF_PDM_InitScriptableField( &m_overburdenPermeability,
                                 "OverburdenPermeability",
                                 10.0e-6,
                                 "Overburden Permeability",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_overburdenFluidDensity,
                                 "OverburdenFluidDensity",
                                 1.03,
                                 "Overburden Fluid Density [g/cm^3]",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableField( &m_underburdenHeight, "UnderburdenHeight", 50.0, "Underburden Height", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_underburdenFormation, "UnderburdenFormation", "Underburden Formation", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_underburdenFacies, "UnderburdenFacies", "Underburden Facies", "", "", "" );
    CAF_PDM_InitScriptableField( &m_underburdenPorosity, "UnderburdenPorosity", 0.0, "Underburden Porosity", "", "", "" );
    CAF_PDM_InitScriptableField( &m_underburdenPermeability,
                                 "UnderburdenPermeability",
                                 10.0e-6,
                                 "Underburden Permeability",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_underburdenFluidDensity,
                                 "UnderburdenFluidDensity",
                                 1.03,
                                 "Underburden Fluid Density [g/cm^3]",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_elasticProperties, "ElasticProperties", "Elastic Properties", "", "", "" );
    m_elasticProperties.uiCapability()->setUiHidden( true );
    m_elasticProperties.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_faciesProperties, "FaciesProperties", "Facies Properties", "", "", "" );
    m_faciesProperties.uiCapability()->setUiHidden( true );
    m_faciesProperties.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_nonNetLayers, "NonNetLayers", "Non-Net Layers", "", "", "" );
    m_nonNetLayers.uiCapability()->setUiHidden( true );
    m_nonNetLayers.uiCapability()->setUiTreeHidden( true );
    setNonNetLayers( new RimNonNetLayers );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelTemplate::~RimStimPlanModelTemplate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimStimPlanModelTemplate::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_overburdenFormation || fieldNeedingOptions == &m_underburdenFormation )
    {
        RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
        if ( !eclipseCaseData ) return options;

        std::vector<QString> formationNames = eclipseCaseData->formationNames();
        for ( const QString& formationName : formationNames )
        {
            options.push_back( caf::PdmOptionItemInfo( formationName, formationName ) );
        }
    }
    else if ( fieldNeedingOptions == &m_overburdenFacies || fieldNeedingOptions == &m_underburdenFacies )
    {
        if ( !m_faciesProperties ) return options;

        RimColorLegend* faciesColors = m_faciesProperties->colorLegend();
        if ( faciesColors )
        {
            for ( RimColorLegendItem* item : faciesColors->colorLegendItems() )
            {
                options.push_back( caf::PdmOptionItemInfo( item->categoryName(), item->categoryName() ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.add( &m_id );

    caf::PdmUiOrdering* defaultsGroup = uiOrdering.addNewGroup( "Defaults" );
    defaultsGroup->add( &m_defaultPorosity );
    defaultsGroup->add( &m_defaultPermeability );

    caf::PdmUiOrdering* referenceStressGroup = uiOrdering.addNewGroup( "Reference Stress" );
    referenceStressGroup->add( &m_verticalStress );
    referenceStressGroup->add( &m_verticalStressGradient );
    referenceStressGroup->add( &m_stressDepth );

    caf::PdmUiOrdering* temperatureGroup = uiOrdering.addNewGroup( "Temperature" );
    temperatureGroup->add( &m_referenceTemperature );
    temperatureGroup->add( &m_referenceTemperatureGradient );
    temperatureGroup->add( &m_referenceTemperatureDepth );

    caf::PdmUiOrdering* overburdenGroup = uiOrdering.addNewGroup( "Overburden" );
    overburdenGroup->add( &m_overburdenHeight );
    overburdenGroup->add( &m_overburdenFormation );
    overburdenGroup->add( &m_overburdenFacies );
    overburdenGroup->add( &m_overburdenPorosity );
    overburdenGroup->add( &m_overburdenPermeability );
    overburdenGroup->add( &m_overburdenFluidDensity );

    caf::PdmUiOrdering* underburdenGroup = uiOrdering.addNewGroup( "Underburden" );
    underburdenGroup->add( &m_underburdenHeight );
    underburdenGroup->add( &m_underburdenFormation );
    underburdenGroup->add( &m_underburdenFacies );
    underburdenGroup->add( &m_underburdenPorosity );
    underburdenGroup->add( &m_underburdenPermeability );
    underburdenGroup->add( &m_underburdenFluidDensity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                      QString                    uiConfigName,
                                                      caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_stressDepth || field == &m_verticalStress )
    {
        auto doubleAttr = dynamic_cast<caf::PdmUiDoubleValueEditorAttribute*>( attribute );
        if ( doubleAttr )
        {
            doubleAttr->m_decimals     = 2;
            doubleAttr->m_numberFormat = caf::PdmUiDoubleValueEditorAttribute::NumberFormat::FIXED;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setId( int id )
{
    m_id = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimStimPlanModelTemplate::id() const
{
    return m_id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticProperties* RimStimPlanModelTemplate::elasticProperties() const
{
    return m_elasticProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setElasticProperties( RimElasticProperties* elasticProperties )
{
    if ( m_elasticProperties )
    {
        m_elasticProperties->changed.disconnect( this );
    }

    m_elasticProperties = elasticProperties;

    if ( m_elasticProperties )
    {
        m_elasticProperties->changed.connect( this, &RimStimPlanModelTemplate::elasticPropertiesChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaciesProperties* RimStimPlanModelTemplate::faciesProperties() const
{
    return m_faciesProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setFaciesProperties( RimFaciesProperties* faciesProperties )
{
    if ( m_faciesProperties )
    {
        m_faciesProperties->changed.disconnect( this );
    }

    m_faciesProperties = faciesProperties;

    if ( m_faciesProperties )
    {
        m_faciesProperties->changed.connect( this, &RimStimPlanModelTemplate::faciesPropertiesChanged );
        RimEclipseCase* eclipseCase = getEclipseCase();
        if ( !eclipseCase ) return;

        m_faciesProperties->setEclipseCase( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::setNonNetLayers( RimNonNetLayers* nonNetLayers )
{
    if ( m_nonNetLayers )
    {
        m_nonNetLayers->changed.disconnect( this );
    }

    m_nonNetLayers = nonNetLayers;

    if ( m_nonNetLayers )
    {
        m_nonNetLayers->changed.connect( this, &RimStimPlanModelTemplate::nonNetLayersChanged );
        RimEclipseCase* eclipseCase = getEclipseCase();
        if ( !eclipseCase ) return;

        m_nonNetLayers->setEclipseCase( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimNonNetLayers* RimStimPlanModelTemplate::nonNetLayers() const
{
    return m_nonNetLayers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::initAfterRead()
{
    RimEclipseCase* eclipseCase = getEclipseCase();
    if ( !eclipseCase ) return;

    if ( m_faciesProperties )
    {
        m_faciesProperties->setEclipseCase( eclipseCase );
    }

    if ( m_nonNetLayers )
    {
        m_nonNetLayers->setEclipseCase( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::faciesPropertiesChanged( const caf::SignalEmitter* emitter )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::elasticPropertiesChanged( const caf::SignalEmitter* emitter )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::nonNetLayersChanged( const caf::SignalEmitter* emitter )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelTemplate::loadDataAndUpdate()
{
    if ( m_elasticProperties )
    {
        m_elasticProperties->loadDataAndUpdate();
    }

    if ( m_faciesProperties )
    {
        m_faciesProperties->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultPorosity() const
{
    return m_defaultPorosity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultPermeability() const
{
    return m_defaultPermeability();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::overburdenFluidDensity() const
{
    return m_overburdenFluidDensity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::underburdenFluidDensity() const
{
    return m_underburdenFluidDensity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::overburdenHeight() const
{
    return m_overburdenHeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::underburdenHeight() const
{
    return m_underburdenHeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultOverburdenPorosity() const
{
    return m_overburdenPorosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultUnderburdenPorosity() const
{
    return m_underburdenPorosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultOverburdenPermeability() const
{
    return m_overburdenPermeability;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::defaultUnderburdenPermeability() const
{
    return m_underburdenPermeability;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModelTemplate::overburdenFormation() const
{
    return m_overburdenFormation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModelTemplate::overburdenFacies() const
{
    return m_overburdenFacies;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModelTemplate::underburdenFormation() const
{
    return m_underburdenFormation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModelTemplate::underburdenFacies() const
{
    return m_underburdenFacies;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::verticalStress() const
{
    return m_verticalStress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::verticalStressGradient() const
{
    return m_verticalStressGradient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::stressDepth() const
{
    return m_stressDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::referenceTemperature() const
{
    return m_referenceTemperature;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::referenceTemperatureGradient() const
{
    return m_referenceTemperatureGradient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::referenceTemperatureDepth() const
{
    return m_referenceTemperatureDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelTemplate::computeDefaultStressDepth()
{
    const double stressDepth = 1000.0;

    RimEclipseCase* eclipseCase = getEclipseCase();
    if ( !eclipseCase ) return stressDepth;

    // Use top of active cells as reference stress depth
    return -eclipseCase->activeCellsBoundingBox().max().z();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStimPlanModelTemplate::getEclipseCase()
{
    // Find an eclipse case
    RimProject* proj = RimProject::current();
    if ( proj->eclipseCases().empty() ) return nullptr;

    return proj->eclipseCases()[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData* RimStimPlanModelTemplate::getEclipseCaseData()
{
    // Find an eclipse case
    RimEclipseCase* eclipseCase = getEclipseCase();
    if ( !eclipseCase ) return nullptr;

    return eclipseCase->eclipseCaseData();
}
