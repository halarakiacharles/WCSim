#include "WCSimDetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4Cons.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4Material.hh"
#include "G4Polycone.hh"
#include "G4Tubs.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"

#include "G4SDManager.hh"
#include "WCSimWCSD.hh"

#include "G4SystemOfUnits.hh"

#include  "WCSimMultiPMTParameterisation.hh"
#include  "G4PVParameterised.hh"

// ex-situ mPMT logical volume construction.
// A function of the WCSimDetectorConstruction class
// dimensions are measured from actual mPMT construction and hard-coded

// 3-inch PMT construction
G4LogicalVolume* WCSimDetectorConstruction::ConstructExSituPMT(G4String PMTName, G4String CollectionName, G4String detectorElement)
{
  G4String keyname = PMTName+"_ExSitu-PMT";
  PMTKey_t key(keyname,CollectionName);
  // Return pre-created PMT Logical Volume if it already exists.
  PMTMap_t::iterator it = PMTLogicalVolumes.find(key);
  if (it != PMTLogicalVolumes.end()) {
    return it->second;
  }

  /////////////////////////////////////////
  // From actual measurements
  G4double pmtGlassThickness = 2.0*mm;
  G4double pmtGlassRadius = 53.0*mm;
  G4double pmtCylRadius = 40.15*mm;
  G4double pmtTubeRadius = 26.1*mm;
  G4double pmtCylHeight = 18.0*mm;
  G4double pmtConeHeight = 20.0*mm;

  G4cout << "========================================================" << G4endl;
  G4cout << "Ex-situ PMT construction: " << G4endl;
  G4cout << "Glass radius for PMT " << CollectionName << " : " << pmtGlassRadius/mm << " mm" << G4endl;
  G4cout << "Cylinder radius : " << pmtCylRadius/mm << " mm" << G4endl;
  G4cout << "========================================================" << G4endl;

  ///////////////////////////////Defining PMT Volume/////////////////////////////////
  G4double zPlanesCylinder[3] = {-20.2, 28.705, 54.4};
  G4double rInnerCylinder[3] = {0, 0, 0};
  G4double rOuterCylinder[3] = {35.22, 44.5, 47.5};
  G4Polycone* solidPMTCyliner = new G4Polycone("solidPMTCyliner",
					  0,
					  360.0*deg,
					  3,
					  zPlanesCylinder,
					  rInnerCylinder,
					  rOuterCylinder);
  
                                                                                                                           
  G4double domeInnerRadius = 332.*mm;
  G4double domeOuterRadius = 347.*mm;

  G4Sphere *domeSphere = new G4Sphere("DomeSphere",
                                      domeInnerRadius,
                                      domeOuterRadius,
                                      0.0*deg, 360.0*deg,
                                      0.0, 90.*deg);

  G4Box *solidBoxCutOut = new G4Box("BoxCutOut",
                                    domeOuterRadius+1.*cm,
                                    domeOuterRadius+1.*cm,
                                    235*mm);

  G4VSolid *domeSolid = new G4SubtractionSolid("domeSolid",
                                               domeSphere,
                                               solidBoxCutOut);
  

  G4VSolid* solidWCPMT = new G4SubtractionSolid("WCPMT",
						      solidPMTCyliner,
						      domeSolid,
						      0,
						      G4ThreeVector(0,0,-277.6*mm));

  G4LogicalVolume *logicWCPMT = new G4LogicalVolume(solidWCPMT,
						    G4Material::GetMaterial("Air1"),
						    "WCPMT");

  //////////////////////////
  // Visualize logicWCPMT //
  //////////////////////////

  if (Vis_Choice == "RayTracer"){
    // Makes the volume containing the PMT visible, solid, and forces the auxiliary edges to be viewed.
    G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
    WCPMTVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
    WCPMTVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown

    logicWCPMT->SetVisAttributes(WCPMTVisAtt);
  } else{
    G4VisAttributes* WCPMTVisAttGrey = new G4VisAttributes(G4Colour(0.2,0.2,0.2));
    WCPMTVisAttGrey->SetForceSolid(true);
    logicWCPMT->SetVisAttributes(G4VisAttributes::Invisible);
    logicWCPMT->SetVisAttributes(WCPMTVisAttGrey);                   
  }

  // inner glass tube
  G4Cons *SolidCone = new G4Cons("pmtCone", 
				 pmtTubeRadius-pmtGlassThickness, 
				 pmtTubeRadius, 
				 pmtCylRadius-pmtGlassThickness, 
				 pmtCylRadius, 
				 pmtConeHeight/2., 
				 0.*deg, 360.*deg);  
  
  G4Tubs *tube = new G4Tubs("tube", 
			    pmtCylRadius-pmtGlassThickness, 
			    pmtCylRadius, 
			    pmtCylHeight/2., 
			    0.*deg, 360.*deg);
  
  G4UnionSolid *combineSolid1 = new G4UnionSolid("innerGlassTube", 
						 SolidCone, 
						 tube, 
						 0, 
						 G4ThreeVector(0.,0.,(pmtConeHeight+pmtCylHeight)/2.));
  
  G4LogicalVolume *logicCombine1 = new G4LogicalVolume(combineSolid1,
						       G4Material::GetMaterial("GlassWCTE"),
						       "logicInnerGlassTube");
  
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,0.),
                    logicCombine1,
                    "InnerGlassTube",
                    logicWCPMT,
                    false,
                    0,
                    checkOverlaps);
  
  // air inside glass tube
  G4Cons *innerSolidCone = new G4Cons("innerPmtCone",
                                0.,
                                pmtTubeRadius-pmtGlassThickness-0.1*mm,
                                0.,
                                pmtCylRadius-pmtGlassThickness-0.1*mm,
                                pmtConeHeight/2.,
                                0.*deg, 360.*deg);

  G4Tubs *innerTube = new G4Tubs("innerTube",
                              0.,
                              pmtCylRadius-pmtGlassThickness,
                              pmtCylHeight/2.,
                              0.*deg, 360.*deg);


  G4UnionSolid *innerCombineSolid = new G4UnionSolid("innerCombineSolid",
                                                 innerSolidCone,
                                                 innerTube,
                                                 0,
                                                 G4ThreeVector(0.,0.,(pmtConeHeight+pmtCylHeight)/2.));

  G4LogicalVolume *innerLogicTube = new G4LogicalVolume(innerCombineSolid, 
							G4Material::GetMaterial("Air1"),
							"logicInnerAir");
  
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,0.0*mm),
                    innerLogicTube,
                    "InnerAir",
                    logicWCPMT,
                    false,
                    0,
                    checkOverlaps);

  //Reflector material inside the PMT
  G4Cons *solidConeTube = new G4Cons("ConeTube",
				 pmtTubeRadius-pmtGlassThickness-0.1*mm,
				 pmtTubeRadius-pmtGlassThickness,
				 pmtCylRadius-pmtGlassThickness-0.1*mm,
				 pmtCylRadius-pmtGlassThickness,
				 pmtConeHeight/2.,
				 0.*deg, 360.*deg);

  G4LogicalVolume *logicLayer = new G4LogicalVolume(solidConeTube, 
						    G4Material::GetMaterial("Aluminum"), 
						    "logicInnerReflector");

  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,0.),
                    logicLayer,
                    "InnerReflector",
                    logicWCPMT,
                    false,
                    0,
                    checkOverlaps);

  new G4LogicalSkinSurface("ReflectorLogSkinSurface", logicLayer, ReflectorSkinSurfaceWCTE);

  G4VisAttributes *layerAttributes = new G4VisAttributes();
  layerAttributes->SetColor(0.85, 0.85, 0.85, 1.0);
  layerAttributes->SetVisibility(true);
  layerAttributes->SetForceSolid(true);
  logicLayer->SetVisAttributes(layerAttributes);
                  
  // artifical absorber at the bottom of glass tube to kill all photons
  G4double absorberThickness = 1.0*mm;
  G4double absorberheight = absorberThickness/2.;
  G4Cons* solidAbsorber = new G4Cons("solidAbsorber", 
				     0.0, 
				     pmtTubeRadius, 
				     0.0, 
				     pmtTubeRadius, 
				     absorberheight, 
				     0.0, 360.0*deg);

  G4ThreeVector absorberPosition(0.,0.,-10.5*mm);
  
  G4LogicalVolume *logicabsorber = new G4LogicalVolume(solidAbsorber, 
				      G4Material::GetMaterial("customAbsorber"),
				      "logicabsorber");
  
  new G4PVPlacement(0, 
                    absorberPosition, 
                    logicabsorber, 
                    "absorber", 
                    logicWCPMT, 
                    false, 
                    0, 
                    checkOverlaps);

  new G4LogicalSkinSurface("AbsorberSurfaceProperties", logicabsorber, AbsorberSkinSurface);
  
  G4VisAttributes *visAttributes = new G4VisAttributes();
  visAttributes->SetColor(G4Color::Gray());
  visAttributes->SetVisibility(true);
  visAttributes->SetForceSolid(true);
  logicabsorber->SetVisAttributes(visAttributes);

  // Photocathode glass
  G4Box *solidCutOff = new G4Box("cutOffTubs",                                                                            
                                 pmtGlassRadius+1.*cm,                                                                     
                                 pmtGlassRadius+1.*cm,                                                                    
                                 34.597*mm);

  G4Sphere *pmtGlassSolid = new G4Sphere("pmtglass", 
					 pmtGlassRadius-pmtGlassThickness, 
					 pmtGlassRadius, 
					 0.0*deg, 360.0*deg, 
					 0.0, 90*deg);  
  
  G4SubtractionSolid *solidGlassFaceWCPMT = new G4SubtractionSolid(CollectionName,                                       
						       pmtGlassSolid,                                   
						       solidCutOff);

  G4LogicalVolume *logicGlassFaceWCPMT = new G4LogicalVolume(solidGlassFaceWCPMT, 
				      G4Material::GetMaterial("GlassWCTE"),
				      CollectionName);
  
  G4VPhysicalVolume *physiGlassFaceWCPMT = new G4PVPlacement(0, 
						   G4ThreeVector(0.,0.,-6.597*mm), 
						   logicGlassFaceWCPMT, 
						   CollectionName, 
						   logicWCPMT, 
						   false, 
						   0, 
						   checkOverlaps);

  if (Vis_Choice == "RayTracer"){
    // Blue wireframe visual style
    // Used in the RayTracer visualizer
    G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
    WCPMTVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
    WCPMTVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown
    //logicGlassFaceWCPMT->SetVisAttributes(G4VisAttributes::Invisible);

    logicGlassFaceWCPMT->SetVisAttributes(WCPMTVisAtt);
  } else {

    // Gray wireframe visual style
    // used in OGLSX visualizer
    //G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.2,0.2,0.2));  //better for seeing hits
    //WCPMTVisAtt->SetForceWireframe(true);
    G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.0,1.0,0.0)); //better for seeing geometry
    WCPMTVisAtt->SetForceSolid(true);
    //logicGlassFaceWCPMT->SetVisAttributes(G4VisAttributes::Invisible);
    logicGlassFaceWCPMT->SetVisAttributes(WCPMTVisAtt);

  }

  // air below photocathode
  G4Sphere *pmtInnerGlassSolid = new G4Sphere("pmtInnerglassSolid",
					      0.*mm,
					      pmtGlassRadius-pmtGlassThickness,
					      0.0*deg, 360.0*deg,
					      0.0, 90*deg);
  G4Box *boxCutOff = new G4Box("cutOffTubs",                                                                            
			       pmtGlassRadius+1.*cm,                                                    
			       pmtGlassRadius+1.*cm,                                             
			       34.597*mm);


  G4SubtractionSolid *solidInteriorWCPMT = new G4SubtractionSolid("InteriorWCPMT",     
							     pmtInnerGlassSolid,
							     boxCutOff); 

  G4LogicalVolume *logicInteriorWCPMT = new G4LogicalVolume(solidInteriorWCPMT,
					  G4Material::GetMaterial("Air1"),
					   "InteriorWCPMT");
  
  G4VPhysicalVolume *physiInteriorWCPMT = new G4PVPlacement(0,
							G4ThreeVector(0.,0.,-6.597*mm),
							logicInteriorWCPMT,
							"InteriorWCPMT",
							logicWCPMT,
							false,
							0,
							checkOverlaps);

  if (Vis_Choice == "RayTracer"){
    // Adding color and forcing the inner portion of the PMT's to be viewed
    G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
    WCPMTVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
    WCPMTVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown

    logicInteriorWCPMT->SetVisAttributes(WCPMTVisAtt);
  } else {
    // Making the inner portion of the detector invisible for OGLSX visualization
    logicInteriorWCPMT->SetVisAttributes(G4VisAttributes::Invisible);
  }

  //Add Logical Border Surface
  new G4LogicalBorderSurface("GlassCathodeSurface",
			     physiGlassFaceWCPMT,
			     physiInteriorWCPMT,
			     OpGlassCathodeSurface);
  // now it is possible for photons to travel from air to cathode
  new G4LogicalBorderSurface("AirCathodeSurface",
			     physiInteriorWCPMT,
           physiGlassFaceWCPMT,
			     OpGlassCathodeSurface);

  // Gel above PMT glass
  G4Sphere *gelSolid = new G4Sphere("GelSolid",
                                    311.95*mm,
                                    332.*mm,
                                    0.0*deg, 360.0*deg,
                                    0.0, 8.02*deg);

  G4Sphere *pmtGelSolid = new G4Sphere("pmtGelSolid",
                                        0.*mm,
                                        pmtGlassRadius,
                                        0.0*deg, 360.0*deg,
                                        0.0, 90.*deg);

  G4Transform3D transform1(G4Translate3D(0,0,(324.03*mm - pmtGlassRadius)));

  G4SubtractionSolid *pmtInnerGelSolid = new G4SubtractionSolid("solidGel",
								gelSolid,                                        
								pmtGelSolid,
								transform1);

  G4LogicalVolume *gelLogic = new G4LogicalVolume(pmtInnerGelSolid,
                                                  G4Material::GetMaterial("SilGel_WCTE"),
                                                  "logicGel");

  new G4PVPlacement(0,                                                                 
                    G4ThreeVector(0.,0.,-277.627*mm),   
                    gelLogic,                                            
                    "Gel",                 
                    logicWCPMT,
                    false,                                                      
                    0,                                                                 
                    checkOverlaps);   

  G4VisAttributes *gelAttributes = new G4VisAttributes();
  gelAttributes->SetColor(1.0, 1.0, 1.0, 0.5);
  gelAttributes->SetVisibility(true);
  gelAttributes->SetForceSolid(true);
  gelLogic->SetVisAttributes(gelAttributes);

  // assume a perfecet smooth surface, so don't need OpGelFoamSurface
  // G4LogicalSkinSurface* gelLogSkinSurface = new G4LogicalSkinSurface("gelLogSkinSurface",gelLogic,OpGelFoamSurface);

  // Plastic cup 

  G4Cons *solidCone1 = new G4Cons("Cone1",
				  27.*mm,
				  27.25*mm,
				  40.55*mm,
				  40.8*mm,
				  10.265*mm,
				  0.*deg, 360.*deg);

  G4Cons *solidCone2 = new G4Cons("Cone2",
                                  43.775*mm,
                                  44.025*mm,
                                  45.38*mm,
                                  45.63*mm,
                                  6.2*mm,
                                  0.*deg, 360.*deg);

  G4Tubs *cupTube = new G4Tubs("cupTube",
			       40.55*mm,
			       40.8*mm,
			       9.22*mm,
			       0.*deg, 360.*deg);
  
  G4UnionSolid *combineCupSolid1 = new G4UnionSolid("tubeplusreflection",
                                                 solidCone1,
                                                 cupTube,
                                                 0,
                                                 G4ThreeVector(0.,0.,19.0*mm));

  G4UnionSolid *combineCupSolid2 = new G4UnionSolid("solidCup",
                                                 combineCupSolid1,
                                                 solidCone2,
                                                 0,
                                                 G4ThreeVector(0.,0.,33.355*mm));


  G4LogicalVolume *logicCupCombine1 = new G4LogicalVolume(combineCupSolid2,                                              
                                                       G4Material::GetMaterial("Plastic"),
                                                       "logicCup");
  
  new G4PVPlacement(0,                                                                         
                    G4ThreeVector(0.,0.,0.*mm),                                               
                    logicCupCombine1,                                                         
                    "PMTCup",                                                              
                    logicWCPMT,                                                         
                    false,                                                                     
                    0,                                                                       
                    checkOverlaps);  
  						
  new G4LogicalSkinSurface("PMTCupSkinSurface",logicCupCombine1, BSSkinSurface); // just use blacksheet surface properties

  G4VisAttributes *cupAttributes = new G4VisAttributes();
  cupAttributes->SetColor(1.0, 0.0, 0.0);
  cupAttributes->SetVisibility(true);
  cupAttributes->SetForceSolid(true);
  logicCupCombine1->SetVisAttributes(cupAttributes);

  // Cup Reflector
  G4Cons *reflectorSolid = new G4Cons("Cone2",
                                      43.505*mm,
                                      43.775*mm,
                                      45.13*mm,
                                      45.38*mm,
                                      5.765*mm,
                                      0.*deg, 360.*deg);

  G4LogicalVolume *reflectorLogic = new G4LogicalVolume(reflectorSolid,
                                                        G4Material::GetMaterial("Aluminum"), 
                                                        "logicReflector");
  
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,34.*mm),
                    reflectorLogic,                                                         
                    "Reflector",                                                       
                    logicWCPMT,                                          
                    false,                                               
                    0,                                               
                    checkOverlaps); 

  new G4LogicalSkinSurface("ReflectorCupSkinSurface",reflectorLogic,ReflectorSkinSurfaceWCTE);

  G4VisAttributes *refAttributes = new G4VisAttributes();
  refAttributes->SetColor(0.85, 0.85, 0.85, 1.0);
  refAttributes->SetVisibility(true);
  refAttributes->SetForceSolid(true);
  reflectorLogic->SetVisAttributes(refAttributes);

  // Poron, filling between PMT and matrix in mPMT 

  G4Tubs *poronSolid = new G4Tubs("poronSolid",
				  26.51*mm,
				  35.22*mm,
				  4.85*mm,
				  0.*deg, 360.*deg);

  G4LogicalVolume *poronLogic = new G4LogicalVolume(poronSolid,
						    G4Material::GetMaterial("PolyurethaneFoam"), 
						    "logicPoron");
    
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,-15.35*mm),  
                    poronLogic,                                                                  
                    "Poron",                                                                     
                    logicWCPMT,                  
                    false,                                                                       
                    0,                                                                           
                    checkOverlaps);         

  new G4LogicalSkinSurface("PoronSurfaceProperties",poronLogic,PoronSkinSurface);                                                              
                                                                                                               
  G4VisAttributes *poronAttributes = new G4VisAttributes();                                                               
  poronAttributes->SetColor(1.0, 1.0, 0.0);                                                                               
  poronAttributes->SetVisibility(true);                                                                                   
  poronAttributes->SetForceSolid(true);                                                                                   
  poronLogic->SetVisAttributes(poronAttributes);          

  /////////////////////////
  // Sensitive detector ///
  /////////////////////////

  // Instantiate a new sensitive detector
  // and register this sensitive detector volume with the SD Manager.
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  G4String SDName = "/WCSim/";
  SDName += CollectionName;

  // If there is no such sensitive detector with that SDName yet,
  // make a new one
  if( ! SDman->FindSensitiveDetector(SDName, false) ) {

    aWCPMT = new WCSimWCSD(CollectionName,SDName,this,detectorElement);
    SDman->AddNewDetector( aWCPMT );
  }

  logicGlassFaceWCPMT->SetSensitiveDetector( aWCPMT );


  PMTLogicalVolumes[key] = logicWCPMT;

  return logicWCPMT;
}

// mPMT construction
G4LogicalVolume* WCSimDetectorConstruction::ConstructExSituMultiPMT(G4String PMTName, G4String CollectionName, G4String detectorElement)
{

  G4bool showme = true;

  G4cout<<"Ex-situ mPMT: Collection Name = "<<CollectionName<<G4endl;

  //unique key for mPMT object. 
  G4String keyname =  mPMT_ID_PMT + "_ExSitu-mPMT";
  PMTKey_t key(keyname,CollectionName);
  
  // Return pre-created mPMT Logical Volume if it already exists.
  PMTMap_t::iterator it = PMTLogicalVolumes.find(key);
  if (it != PMTLogicalVolumes.end()) {
    return it->second;
  }

  G4double domeInnerRadius = 332.*mm;
  G4double domeOuterRadius = 347.*mm;
  G4double domeCut = 235*mm;

  G4double vessel_cylinder_height = 77.785*2*mm;
  G4double vessel_inner_radius = 225.6*mm;
  G4double vessel_outer_radius = 254.*mm;
  G4double vessel_cap_height = domeOuterRadius - domeCut;

  G4double offsetFromBox = 198.17*mm; // offset from original code where world volume is a box

  G4int nIDPMTs = 19;
  
  G4cout << "Create ex-situ mPMT" <<  G4endl;
  G4cout << "Vessel height: " << vessel_cylinder_height << G4endl;
  G4cout << "Vessel radius: " << vessel_outer_radius << G4endl;
  G4cout << "Vessel radius curv: " << domeOuterRadius << G4endl;
  G4cout << "Vessel cap height: " << vessel_cap_height << G4endl;
  G4cout << "Number of ID PMTs: " << nIDPMTs << G4endl;

  //All components of the PMT are now contained in a single logical volume logicWCPMT.
  //Origin is cylinder bottom, faces positive z-direction.

  ////////////////////////////////////////////////////
  /// 1) Outer logical volume: fill with air     ///
  ////////////////////////////////////////////////////
  G4double mPMT_zRange_outer[2] = {0,                         // start from zero for easier placement in ID
				   vessel_cylinder_height};
  G4double mPMT_RRange_outer[2] = {vessel_outer_radius, 
				   vessel_outer_radius};
  G4double mPMT_rRange_outer[2] = {0., 0.};

  // Although G4Tubs is more natural, Polycone is used to be in control of z position
  // and because z = 0 is position of bottom of cylinder, vs center in G4Tubs
  G4Polycone* solidMultiPMT_vessel = 
    new G4Polycone("WCmPMT_vessel",                    
		   0.0*deg,
		   360.0*deg,
		   2,
		   mPMT_zRange_outer,
		   mPMT_rRange_outer, // R Inner
		   mPMT_RRange_outer);// R Outer
  
  G4Sphere* mPMT_top_sphere =
    new G4Sphere(    "WCmPMT_tsphere",
		     0,
		     domeOuterRadius,
		     0.0*deg,360.0*deg,
		     0.0*deg,90.0*deg);
  
  G4Box*  solidCutOffTubs = 
    new G4Box(    "cutOffTubs",
      domeOuterRadius + 1.*cm,                      // x/2
      domeOuterRadius + 1.*cm,                      // y/2
      domeCut);         // z/2

  G4SubtractionSolid * mPMT_top_cap_vessel = new G4SubtractionSolid("WCmPMT_tsphere_vessel",
                                                                    mPMT_top_sphere,
                                                                    solidCutOffTubs);

  G4UnionSolid *solidMultiPMT = 
	  new G4UnionSolid("WCmPMT",solidMultiPMT_vessel,mPMT_top_cap_vessel,0,G4ThreeVector(0,0,vessel_cylinder_height-domeCut));

  G4LogicalVolume *logicWCMultiPMT =
    new G4LogicalVolume(    solidMultiPMT,
			    G4Material::GetMaterial("Air1"), 
			    "WCMultiPMT",
			    0,0,0);



  ////////////////////////////////////////////////
  /// 2)Top acrylic + shell of the mPMT vessel ///
  ////////////////////////////////////////////////
  G4Sphere *domeSphere = new G4Sphere("DomeSphere",
				      domeInnerRadius,
				      domeOuterRadius,
				      0.0*deg, 360.0*deg,
				      0.0, 90.*deg);

  G4VSolid *domeSolid = new G4SubtractionSolid("domeSolid",
                                               domeSphere,
                                               solidCutOffTubs);

  G4LogicalVolume *domeLogic = new G4LogicalVolume(domeSolid,
                                                   G4Material::GetMaterial("G4_PLEXIGLASS"),
                                                   "logicDome");
  
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,vessel_cylinder_height-domeCut),
                    domeLogic,
                    "physDome",
                    logicWCMultiPMT,
                    false,
                    0,
                    checkOverlaps);

  G4VisAttributes *domeAttributes = new G4VisAttributes();
  domeAttributes->SetColor(1.0, 1.0, 1.0, 0.5);
  domeAttributes->SetVisibility(true);
  domeAttributes->SetForceSolid(true);
  domeLogic->SetVisAttributes(domeAttributes);

  G4Tubs *cylinderSolid = new G4Tubs("cylinderSolid",
				     vessel_inner_radius,
				     vessel_outer_radius,
				     vessel_cylinder_height/2,
				     0.*deg, 360.*deg);

  G4LogicalVolume *cylinderLogic = new G4LogicalVolume( cylinderSolid,
                                                        G4Material::GetMaterial("Plastic"),
                                                        "logicCylinder");
  
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,vessel_cylinder_height/2),
                    cylinderLogic,
                    "physCylinder",
                    logicWCMultiPMT,
                    false,
                    0,
                    checkOverlaps);

  new G4LogicalSkinSurface("cylinderSkinSurface", cylinderLogic, BSSkinSurface); // assume same as blacksheet
      
  G4VisAttributes *cylinderAttributes = new G4VisAttributes();  
  cylinderAttributes->SetColor(0.2, 0.2, 0.2, 1.0);  
  cylinderAttributes->SetVisibility(true); 
  cylinderAttributes->SetForceSolid(true);  
  cylinderLogic->SetVisAttributes(cylinderAttributes);

  /////////////////////////////////////////////////////////////////////////
  /// 3) This is the area between the outer shell and the inner shell.  ///
  ///    In this space, the PMTs will live.                             ///
  ///    Add the supporting matrix and flange here                      ///
  /////////////////////////////////////////////////////////////////////////

  G4Sphere *matrixSolid = new G4Sphere("MatrixSolid",
				       264.24*mm,
				       270.24*mm,
				       0.0*deg, 360.0*deg,
				       0.0, 44.336*deg);
  

  G4LogicalVolume *matrixLogic = new G4LogicalVolume(matrixSolid,
						     G4Material::GetMaterial("Plastic"),
						     "logicMatrix");
   
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,-299.*mm + offsetFromBox),
                    matrixLogic,
                    "physMatrix",
                    logicWCMultiPMT,
                    false,
                    0,
                    checkOverlaps);  

  new G4LogicalSkinSurface("matrixSkinSurface", matrixLogic, BSSkinSurface); // assume same as blacksheet

  G4Tubs *flangeSolid = new G4Tubs("flangeSolid",
                                    185.*mm,
                                    223.*mm,
                                    3.*mm,
                                    0.*deg, 360.*deg);

  G4LogicalVolume *flangeLogic = new G4LogicalVolume(flangeSolid,
						                                          G4Material::GetMaterial("Plastic"),
                                                     "logicFlange");
  
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,-112.85*mm + offsetFromBox),
                    flangeLogic,
                    "physFlange",
                    logicWCMultiPMT,
                    false,
                    0,
                    checkOverlaps);

  new G4LogicalSkinSurface("flangeSkinSurface", flangeLogic, BSSkinSurface); // assume same as blacksheet

  /////////////////////////////////////////////////////
  /// 4) Fill the mPMT mpmt_vessel with single (ID) PMTs ///
  /////////////////////////////////////////////////////
  
  G4LogicalVolume* logicWCPMT = ConstructExSituPMT(PMTName, CollectionName,detectorElement);
  
  G4double xArray[19] ={0.*mm, 84.43*mm, 42.21*mm, -42.21*mm, -84.43*mm, -42.21*mm, 42.21*mm, 155.109*mm, 134.32*mm, 77.55*mm, 0.*mm, -77.55*mm, -134.32*mm, -155.109*mm, -134.32*mm, -77.55*mm, 0.*mm, 77.55*mm, 134.32*mm};

  G4double yArray[19] = {0.*mm, 0.*mm, -73.125*mm, -73.125*mm, 0.*mm, 73.125*mm, 73.125*mm, 0.*mm, -77.55*mm, -134.32*mm, -155.109*mm, -134.32*mm, -77.55*mm, 0.*mm, 77.55*mm, 134.32*mm, 155.109*mm, 134.32*mm, 77.55*mm};

  G4double zArray[19] = {0.*mm, -13.91*mm, -13.91*mm, -13.91*mm, -13.91*mm, -13.91*mm, -13.91*mm, -50.88*mm, -50.88*mm, -50.88*mm, -50.88*mm, -50.88*mm, -50.88*mm, -50.88*mm, -50.88*mm, -50.88*mm, -50.88*mm, -50.88*mm, -50.88*mm};

  G4double xthetaArray[19] = {0.*deg, 0.*deg, 16.24*deg, 16.24*deg, 0.*deg, -16.24*deg, -16.24*deg, 0.*deg, 19.861*deg, 32.03*deg, 35.846*deg, 32.03*deg, 19.861*deg, 0.*deg, -19.861*deg, -32.03*deg, -35.846*deg, -32.03*deg, -19.861*deg};

  G4double ythetaArray[19] = {0.*deg, 18.59*deg, 9.54*deg, -9.54*deg, -18.59*deg, -9.54*deg, 9.54*deg, 35.84*deg, 32.03*deg, 19.861*deg, 0.*deg, -19.861*deg, -32.03*deg, -35.846*deg, -32.03*deg, -19.861*deg, 0.*deg, 19.861*deg, 32.03*deg};

  G4double phiArray[19] = {0.*deg, 0.*deg, -60.*deg, -120.*deg, 180.*deg, 120.*deg, 60*deg, 0.*deg, -30.*deg, -60.*deg, -90.*deg, -120.*deg, -150.*deg, 180.*deg, 150.*deg, 120.*deg, 90.*deg, 60.*deg, 30.*deg};


  for (int i = 0; i < nIDPMTs; i++) {
    G4double x = xArray[i];
    G4double y = yArray[i];
    G4double z = zArray[i] + offsetFromBox ;

    G4double xtheta = xthetaArray[i];
    G4double ytheta = ythetaArray[i];
    G4double phi = phiArray[i];

    G4RotationMatrix* rot = new G4RotationMatrix();
    rot->rotateX(-xtheta);
    rot->rotateY(-ytheta);
    rot->rotateZ(phi);

    G4ThreeVector translation(x,y,z);
    
    // Create and place the PMT copy                                                                                    
    new G4PVPlacement(rot,
                      translation,
                      logicWCPMT,
                      "pmt",
                      logicWCMultiPMT,
                      false,
                      i,
                      checkOverlaps);
    
  }  
 
  /* Set all visualization here for better overview. */
  // Gray wireframe visual style
  G4VisAttributes* VisAttGrey = new G4VisAttributes(G4Colour(0.2,0.2,0.2));
  //VisAttGrey->SetForceWireframe(true);
  VisAttGrey->SetForceSolid(true); 
  G4VisAttributes* WCPMTVisAtt5 = new G4VisAttributes(G4Colour(.0,1.,1.));
  WCPMTVisAtt5->SetForceSolid(true); 
  G4VisAttributes* VisAttYellow = new G4VisAttributes(G4Colour(1.0,1.,0.));
  VisAttYellow->SetForceSolid(true); 
  
  if(showme){
    logicWCMultiPMT->SetVisAttributes(VisAttGrey); 
    cylinderLogic->SetVisAttributes(WCPMTVisAtt5);    
    flangeLogic->SetVisAttributes(VisAttYellow); 
  }

  // Keep track of already created mPMT logical volumes in same map as for PMTs
  PMTLogicalVolumes[key] = logicWCMultiPMT;

  return logicWCMultiPMT;
}
