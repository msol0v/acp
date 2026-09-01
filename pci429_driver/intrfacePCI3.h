// intrfacePCI3.h
//
// Define an Interface Guid for pci429_3win device class.
// This GUID is used to register (IoRegisterDeviceInterface)
// an instance of an interface so that user application
// can control the pci429_3win device.
//
//  {16A93998-C724-41C4-A466-61A5106E3A29}
DEFINE_GUID(GUID_DEVINTERFACE_PCI429_3WIN,
    0x16A93998, 0xC724, 0x41C4, 0xA4, 0x66, 0x61, 0xA5, 0x10, 0x6E, 0x3A, 0x29);

// Define a Setup Class GUID for pci429_3win Class. This is same
// as the PCI429_3WIN CLASS guid in the INF files.
//
//  {90493237-49DC-48E6-AD47-208BB8EE8BF7}
DEFINE_GUID(GUID_DEVCLASS_PCI429_3WIN,
 	0x79706367, 0x88c8, 0x4ca5, 0x8d, 0x45, 0x45, 0xc3, 0x32, 0xc5, 0x15, 0x28);
//   0x90493237, 0x49DC, 0x48E6, 0xAD, 0x47, 0x20, 0x8B, 0xB8, 0xEE, 0x8B, 0xF7);


// GUID definition are required to be outside of header inclusion pragma to avoid
// error during precompiled headers.
//

