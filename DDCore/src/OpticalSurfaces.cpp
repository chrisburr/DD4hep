//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================

// Framework include files
#include "DD4hep/OpticalSurfaces.h"
#include "DD4hep/NamedObject.h"
#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"
#include "DD4hep/World.h"

#include "DD4hep/detail/Handle.inl"

// C/C++ includes
#include <sstream>
#include <iomanip>

using namespace std;
using namespace dd4hep;

#if ROOT_VERSION_CODE > ROOT_VERSION(6,16,0)

DD4HEP_INSTANTIATE_HANDLE(TGeoSkinSurface);
DD4HEP_INSTANTIATE_HANDLE(TGeoBorderSurface);
DD4HEP_INSTANTIATE_HANDLE(TGeoOpticalSurface);

/// Initializing constructor.
OpticalSurface::OpticalSurface(Detector& description,
                               const string& nam,
                               EModel  model,
                               EFinish finish,
                               EType   type,
                               double  value)
{
  unique_ptr<Object> s(new Object(nam.c_str(), model, finish, type, value));
  description.manager().AddOpticalSurface(m_element=s.release());
}

/// Initializing constructor: Creates the object and registers it to the manager
SkinSurface::SkinSurface(DetElement de, const string& nam, OpticalSurface surf, Volume vol)
{
  if ( de.isValid() )  {
    string n = de.path() + '#' + nam;
    if ( vol.isValid() )  {
      if ( surf.isValid() )   {
        World wrld = de.world();
        unique_ptr<Object> s(new Object(n.c_str(), surf->GetName(), surf.ptr(), vol.ptr()));
        wrld.detectorDescription().manager().AddSkinSurface(m_element=s.release());
        return;
      }
      except("SkinSurface","++ Cannot create SkinSurface %s without valid optical surface!",n.c_str());
    }
    except("SkinSurface","++ Cannot create SkinSurface %s without valid volume!",n.c_str());
  }
  except("SkinSurface",
         "++ Cannot create SkinSurface %s which is not connected to a valid detector element!",nam.c_str());
}

/// Access surface data
OpticalSurface SkinSurface::surface()  const    {
  return (TGeoOpticalSurface*)(access()->GetSurface());
}

/// Access the node of the skin surface
Volume   SkinSurface::volume()   const    {
  return access()->GetVolume();
}

/// Initializing constructor: Creates the object and registers it to the manager
BorderSurface::BorderSurface(DetElement de, const string& nam, OpticalSurface surf, PlacedVolume lft, PlacedVolume rht)
{
  if ( de.isValid() )  {
    string n = de.path() + '#' + nam;
    if ( lft.isValid() && rht.isValid() )  {
      if ( surf.isValid() )   {
        World wrld = de.world();
        unique_ptr<Object> s(new Object(n.c_str(), surf->GetName(), surf.ptr(), lft.ptr(), rht.ptr()));
        wrld.detectorDescription().manager().AddBorderSurface(m_element=s.release());
        return;
      }
      except("BorderSurface","++ Cannot create BorderSurface %s without valid optical surface!",n.c_str());
    }
    except("BorderSurface","++ Cannot create BorderSurface %s without valid placements!",n.c_str());
  }
  except("BorderSurface",
         "++ Cannot create BorderSurface %s which is not connected to a valid detector element!",nam.c_str());
}

/// Access surface data
OpticalSurface BorderSurface::surface()  const    {
  return (TGeoOpticalSurface*)(access()->GetSurface());
}

/// Access the left node of the border surface
PlacedVolume   BorderSurface::left()   const    {
  return (TGeoNode*)access()->GetNode1();
}

/// Access the right node of the border surface
PlacedVolume   BorderSurface::right()  const    {
  return access()->GetNode2();
}
#endif
