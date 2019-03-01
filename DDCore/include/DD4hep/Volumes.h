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
#ifndef DD4HEP_DDCORE_VOLUMES_H
#define DD4HEP_DDCORE_VOLUMES_H

// Framework include files
#include "DD4hep/Handle.h"
#include "DD4hep/Shapes.h"
#include "DD4hep/Objects.h"

// C/C++ include files
#include <map>

// ROOT include file (includes TGeoVolume + TGeoShape)
#include "TGeoNode.h"
#include "TGeoPatternFinder.h"

#if ROOT_VERSION_CODE > ROOT_VERSION(5,34,9)
// Recent ROOT versions
#include "TGeoExtension.h"

#else
// Older ROOT version
#define DD4HEP_EMULATE_TGEOEXTENSIONS
class TGeoExtension : public TObject  {
public:
  virtual ~TGeoExtension() {}
  /// TGeoExtension overload: Method called whenever requiring a pointer to the extension
  virtual TGeoExtension *Grab() = 0;
  /// TGeoExtension overload: Method called always when the pointer to the extension is not needed anymore
  virtual void Release() const = 0;
};
#endif

/// Namespace for the AIDA detector description toolkit
namespace dd4hep {

  class  Detector;
  class  Region;
  class  LimitSet;
  class  Material;
  class  VisAttr;
  class  DetElement;
  class  SensitiveDetector;

  // Forward declarations
  class  Volume;
  class  PlacedVolume;

  /// Implementation class extending the ROOT placed volume
  /**
   *   For any further documentation please see the following ROOT documentation:
   *   \see http://root.cern.ch/root/html/TGeoExtension.html
   *
   *   \author  M.Frank
   *   \version 1.0
   *   \ingroup DD4HEP_CORE
   */
  class PlacedVolumeExtension : public TGeoExtension  {
  public:
    typedef std::pair<std::string, int> VolID;
    /// Volume ID container
    /**
     *   \author  M.Frank
     *   \version 1.0
     *   \ingroup DD4HEP_CORE
     */
    class VolIDs: public std::vector<VolID> {
    public:
      typedef std::vector<VolID> Base;
      /// Default constructor
      VolIDs() = default;
      /// Copy constructor
      VolIDs(const VolIDs& c) = default;
      /// Destructor
      ~VolIDs() {}
      /// Assignment operator        
      VolIDs& operator=(const VolIDs& c) = default;
      /// Find entry
      std::vector<VolID>::const_iterator find(const std::string& name) const;
      /// Insert new entry
      std::pair<std::vector<VolID>::iterator, bool> insert(const std::string& name, int value);
      /// Insert bunch of entries
      template< class InputIt>
      iterator insert(InputIt first, InputIt last)
      {  return this->Base::insert(this->Base::end(), first, last);    }
#if !defined __GNUCC__ || (defined __GNUCC__ && __GNUC__ > 5)
      /// Insert bunch of entries
      template< class InputIt>
      iterator insert(std::vector<VolID>::const_iterator pos, InputIt first, InputIt last)
      {  return this->Base::insert(pos, first, last);    }
#endif
      /// String representation for debugging
      std::string str()  const;
    };
    /// Magic word to detect memory corruptions
    unsigned long magic = 0;
    /// Reference count on object (used to implement Grab/Release)
    long   refCount     = 0;
    /// ID container
    VolIDs volIDs;
    /// Default constructor
    PlacedVolumeExtension();
    /// Copy constructor
    PlacedVolumeExtension(const PlacedVolumeExtension& c);
    /// Default destructor
    virtual ~PlacedVolumeExtension();
    /// Assignment operator
    PlacedVolumeExtension& operator=(const PlacedVolumeExtension& c) {
      magic = c.magic;
      volIDs = c.volIDs;
      return *this;
    }
    /// TGeoExtension overload: Method called whenever requiring a pointer to the extension
    virtual TGeoExtension *Grab()  override;
    /// TGeoExtension overload: Method called always when the pointer to the extension is not needed anymore
    virtual void Release() const  override;
    /// Enable ROOT persistency
    ClassDefOverride(PlacedVolumeExtension,200);
  };

  /// Handle class holding a placed volume (also called physical volume)
  /**
   *   For any further documentation please see the following ROOT documentation:
   *   \see http://root.cern.ch/root/html/TGeoNode.html
   *
   *   \author  M.Frank
   *   \version 1.0
   *   \ingroup DD4HEP_CORE
   */
  class PlacedVolume : public Handle<TGeoNode> {
  public:
    typedef PlacedVolumeExtension         Object;
    typedef PlacedVolumeExtension::VolIDs VolIDs;

    /// Abstract base for processing callbacks to PlacedVolume objects
    /** Helper to facilitate building plugins, which instrument
     *  placements and volumes e.g. during geometry scans.
     *
     *  \author  M.Frank
     *  \version 1.0
     *  \ingroup DD4HEP_CORE
     */
    class Processor {
    public:
      /// Default constructor
      Processor();
      /// Default destructor
      virtual ~Processor();
      /// Container callback for object processing
      virtual int processPlacement(PlacedVolume pv) = 0;
    };

    /// Default constructor
    PlacedVolume() = default;
    /// Copy assignment
    PlacedVolume(const PlacedVolume& e) = default;
    /// Copy assignment from other handle type
    template <typename T> PlacedVolume(const Handle<T>& e) : Handle<TGeoNode>(e) {  }
    /// Constructor taking implementation object pointer
    PlacedVolume(const TGeoNode* e) : Handle<TGeoNode>(e) {  }
    /// Assignment operator (must match copy constructor)
    PlacedVolume& operator=(const PlacedVolume& v)  = default;
    /// Check if placement is properly instrumented
    Object* data() const;
    /// Add identifier
    PlacedVolume& addPhysVolID(const std::string& name, int value);
    /// Volume material
    Material material() const;
    /// Logical volume of this placement
    Volume volume() const;
    /// Parent volume (envelope)
    Volume motherVol() const;
    /// Access the full transformation matrix to the parent volume
    const TGeoMatrix& matrix()  const;
    /// Access the translation vector to the parent volume
    Position position()  const;
    /// Access to the volume IDs
    const PlacedVolumeExtension::VolIDs& volIDs() const;
    /// String dump
    std::string toString() const;
  };

  /// Implementation class extending the ROOT volume (TGeoVolume)
  /**
   *   Internal data structure optional to TGeo data.
   *
   *   For any further documentation please see the following ROOT documentation:
   *   \see http://root.cern.ch/root/html/TGeoExtension.html
   *
   *   \author  M.Frank
   *   \version 1.0
   *   \ingroup DD4HEP_CORE
   */
  class VolumeExtension : public TGeoExtension {
  public:
    /// Magic word to detect memory corruptions
    unsigned long       magic      = 0;
    /// Reference count on object (used to implement Grab/Release)
    long                refCount   = 0;
    ///
    int                 referenced = 0;
    /// Bit field to determine the usage. Bit 0...15 reserverd for system usage. 16...31 user space.
    int                 flags      = 0;
    /// Region reference
    Region              region;
    /// Limit sets used for simulation
    LimitSet            limits;
    /// Reference to visualization attributes
    VisAttr             vis;
    /// Reference to the sensitive detector
    Handle<NamedObject> sens_det;
    
    /// Default constructor
    VolumeExtension();
    /// Default destructor
    virtual ~VolumeExtension();
    /// Copy the object
    void copy(const VolumeExtension& c) {
      magic      = c.magic;
      region     = c.region;
      limits     = c.limits;
      vis        = c.vis;
      sens_det   = c.sens_det;
      referenced = c.referenced;
    }
    /// TGeoExtension overload: Method called whenever requiring a pointer to the extension
    virtual TGeoExtension *Grab()  override;
    /// TGeoExtension overload: Method called always when the pointer to the extension is not needed anymore
    virtual void Release() const  override;
    /// Enable ROOT persistency
    ClassDefOverride(VolumeExtension,200);
  };

  /// Handle class holding a placed volume (also called physical volume)
  /**
   *   Handle describing a Volume
   *
   *   One note about divisions:
   *   =========================
   *   Since dd4hep requires Volumes (aka TGeoVolume) and PlacedVolumes (aka TGeoNode)
   *   to be enhaced with the user extension mechanism shape divisions MUST be
   *   done using the division mechanism of the dd4hep shape or volume wrapper.
   *   Otherwise the enhancements are not added and you shall get exception
   *   when dd4hep is closing the geometry.
   *   The same argument holds when a division is made from a Volume.
   *   Unfortunately there is no reasonable way to intercept this call to the
   *   TGeo objects - except to sub-class each of them, which is not really 
   *   acceptable either.
   *
   *   For any further documentation please see the following ROOT documentation:
   *   \see http://root.cern.ch/root/html/TGeoVolume.html
   *
   *   \author  M.Frank
   *   \version 1.0
   *   \ingroup DD4HEP_CORE
   */
  class Volume: public Handle<TGeoVolume> {
  public:
    typedef VolumeExtension Object;
    /// Flag bit numbers for special volume treatments
    enum  {
      VETO_SIMU     = 1,
      VETO_RECO     = 2,
      VETO_DISPLAY  = 3
    };
  public:
    /// Default constructor
    Volume() = default;
    /// Copy from handle
    Volume(const Volume& v) = default;
    /// Copy from handle
    Volume(const TGeoVolume* v) : Handle<TGeoVolume>(v) { }
    /// Copy from arbitrary Element
    template <typename T> Volume(const Handle<T>& v) : Handle<TGeoVolume>(v) {  }

    /// Constructor to be used when creating a new geometry tree.
    Volume(const std::string& name);

    /// Constructor to be used when creating a new geometry tree. Also sets materuial and solid attributes
    Volume(const std::string& name, const Solid& s, const Material& m);

    /// Assignment operator (must match copy constructor)
    Volume& operator=(const Volume& a)  = default;

    /// Check if placement is properly instrumented
    Object* data() const;

    /// If we import volumes from external sources, we have to attach the extensions to the tree
    Volume& import();

    /// Divide volume into subsections (See the ROOT manuloa for details)
    Volume divide(const std::string& divname, int iaxis, int ndiv, double start, double step, int numed = 0, const char* option = "");
    /** Daughter placements with auto-generated copy number for the daughter volume  */
    /// Place daughter volume. The position and rotation are the identity
    PlacedVolume placeVolume(const Volume& vol) const;
    /// Place daughter volume according to a generic Transform3D
    PlacedVolume placeVolume(const Volume& volume, const Transform3D& tr) const;
    /// Place un-rotated daughter volume at the given position.
    PlacedVolume placeVolume(const Volume& vol, const Position& pos) const;
    /// Place rotated daughter volume. The position is automatically the identity position
    PlacedVolume placeVolume(const Volume& vol, const RotationZYX& rot) const;
    /// Place rotated daughter volume. The position is automatically the identity position
    PlacedVolume placeVolume(const Volume& vol, const Rotation3D& rot) const;

    /** Daughter placements with user supplied copy number for the daughter volume  */
    /// Place daughter volume. The position and rotation are the identity
    PlacedVolume placeVolume(const Volume& vol, int copy_no) const;
    /// Place daughter volume according to a generic Transform3D
    PlacedVolume placeVolume(const Volume& volume, int copy_no, const Transform3D& tr) const;
    /// Place un-rotated daughter volume at the given position.
    PlacedVolume placeVolume(const Volume& vol, int copy_no, const Position& pos) const;
    /// Place rotated daughter volume. The position is automatically the identity position
    PlacedVolume placeVolume(const Volume& vol, int copy_no, const RotationZYX& rot) const;
    /// Place rotated daughter volume. The position is automatically the identity position
    PlacedVolume placeVolume(const Volume& vol, int copy_no, const Rotation3D& rot) const;
    /// Parametrized volume implementation
    /** Embedding parametrized daughter placements in a mother volume
     *  @param start  start transormation for the first placement
     *  @param count  Number of entities to be placed
     *  @param entity Daughter volume to be placed
     *  @param inc    Transformation increment for each iteration
     */
    void paramVolume1D(const Transform3D& start, size_t count, Volume entity, const Transform3D& inc);
    /// Parametrized volume implementation
    /** Embedding parametrized daughter placements in a mother volume
     *  @param count  Number of entities to be placed
     *  @param entity Daughter volume to be placed
     *  @param inc    Transformation increment for each iteration
     */
    void paramVolume1D(size_t count, Volume entity, const Transform3D& trafo);
    /// Parametrized volume implementation
    /** Embedding parametrized daughter placements in a mother volume
     *  @param count  Number of entities to be placed
     *  @param entity Daughter volume to be placed
     *  @param inc    Transformation increment for each iteration
     */
    void paramVolume1D(size_t count, Volume entity, const Position& inc);
    /// Parametrized volume implementation
    /** Embedding parametrized daughter placements in a mother volume
     *  @param count  Number of entities to be placed
     *  @param entity Daughter volume to be placed
     *  @param inc    Transformation increment for each iteration
     */
    void paramVolume1D(size_t count, Volume entity, const RotationZYX& inc);

    /// Set user flags in bit-field
    void setFlagBit(unsigned int bit);
    /// Test the user flag bit
    bool testFlagBit(unsigned int bit)   const;
    
    /// Attach attributes to the volume
    const Volume& setAttributes(const Detector& description, const std::string& region, const std::string& limits,
                                const std::string& vis) const;

    /// Set the regional attributes to the volume. Note: If the name string is empty, the action is ignored.
    const Volume& setRegion(const Detector& description, const std::string& name) const;
    /// Set the regional attributes to the volume
    const Volume& setRegion(const Region& obj) const;
    /// Access to the handle to the region structure
    Region region() const;

    /// Set the limits to the volume. Note: If the name string is empty, the action is ignored.
    const Volume& setLimitSet(const Detector& description, const std::string& name) const;
    /// Set the limits to the volume
    const Volume& setLimitSet(const LimitSet& obj) const;
    /// Access to the limit set
    LimitSet limitSet() const;

    /// Set Visualization attributes to the volume
    const Volume& setVisAttributes(const VisAttr& obj) const;
    /// Set Visualization attributes to the volume. Note: If the name string is empty, the action is ignored.
    const Volume& setVisAttributes(const Detector& description, const std::string& name) const;
    /// Access the visualisation attributes
    VisAttr visAttributes() const;

    /// Assign the sensitive detector structure
    const Volume& setSensitiveDetector(const SensitiveDetector& obj) const;
    /// Access to the handle to the sensitive detector
    Handle<NamedObject> sensitiveDetector() const;
    /// Accessor if volume is sensitive (ie. is attached to a sensitive detector)
    bool isSensitive() const;

    /// Set the volume's solid shape
    const Volume& setSolid(const Solid& s) const;
    /// Access to Solid (Shape)
    Solid solid() const;
    /// Access the bounding box of the volume (if available)
    Box boundingBox() const;

    /// Set the volume's material
    const Volume& setMaterial(const Material& m) const;
    /// Access to the Volume material
    Material material() const;

    /// Auto conversion to underlying ROOT object
    operator TGeoVolume*() const {
      return m_element;
    }
  };

  /// Implementation class extending the ROOT mulit-volumes (TGeoVolumeMulti)
  /**
   *  Handle describing a multi volume.
   *
   *   For any further documentation please see the following ROOT documentation:
   *   \see http://root.cern.ch/root/html/TGeoVolumeMulti.html
   *
   *   \author  M.Frank
   *   \version 1.0
   *   \ingroup DD4HEP_CORE
   */
  class VolumeMulti : public Volume   {
    void verifyVolumeMulti();
  public:
    /// Default constructor
    VolumeMulti() = default;
    /// Copy from handle
    VolumeMulti(const VolumeMulti& v) = default;
    /// Copy from pointer as a result of Solid->Divide()
    VolumeMulti(TGeoVolume* v) : Volume(v)  {
      verifyVolumeMulti();
    }
    /// Copy from arbitrary Element
    template <typename T> VolumeMulti(const Handle<T>& v) : Volume(v) {
      verifyVolumeMulti();
    }
    /// Constructor to be used when creating a new multi-volume object
    VolumeMulti(const std::string& name, Material material);
    /// Assignment operator (must match copy constructor)
    VolumeMulti& operator=(const VolumeMulti& a) = default;
  };

  /// Implementation class extending the ROOT assembly volumes (TGeoVolumeAssembly)
  /**
   *  Handle describing a volume assembly.
   *
   *   For any further documentation please see the following ROOT documentation:
   *   \see http://root.cern.ch/root/html/TGeoVolumeAssembly.html
   *
   *   \author  M.Frank
   *   \version 1.0
   *   \ingroup DD4HEP_CORE
   */
  class Assembly: public Volume {
  public:
    /// Default constructor
    Assembly() = default;
    /// Copy from handle
    Assembly(const Assembly& v) = default;
    /// Copy from arbitrary Element
    template <typename T> Assembly(const Handle<T>& v) : Volume(v) {  }
    /// Constructor to be used when creating a new assembly object
    Assembly(const std::string& name);
    /// Assignment operator (must match copy constructor)
    Assembly& operator=(const Assembly& a) = default;
  };

  /// Output mesh vertices to string
  std::string toStringMesh(PlacedVolume solid, int precision=2);
}         /* End namespace dd4hep          */
#endif    /* DD4HEP_DDCORE_VOLUMES_H       */
