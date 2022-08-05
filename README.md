**Payload to mount SD and/or EMMC via USB.**

Menu should be pretty self explanatory, allows to mount SD Card and all EMMC Partitions in read-only or read/write mode.  

It does not depend on any files on the SD card (unlike e.g. hekate which needs nyx to be present on SD card), just inject the payload and select the volumes you want to mount.
  
The payload only uses IRAM. This may be very helpful when your switch has broken RAM. 
  
Payload can be configured by writing a configuration to the following offsets:  
  
Offset: 0x94  
  
| Bit | Default | Function                                                                                                          |
|-----|---------|-------------------------------------------------------------------------------------------------------------------|
| 0   | 0       | 0: Show the interactive Menu                                                                                      |
|     |         | 1: Start UMS automatically, use config in 0xa5                                                                    |
| 1:2 | 0       | 0: Return to menu after UMS has stopped 1: Power off after UMS has stopped 2: Reboot to RCM after UMS has stopped |

Offset: 0x95  
  
| Bit | Default | Function                           |
|-----|---------|------------------------------------|
| 0:1 | 2       | 0: Don't mount SD                  |
|     |         | 1: Mount SD as read-only           |
|     |         | 2: Mount SD as read/write          |
| 2:3 | 0       | 0: Don't mount EMMC-GPP            |
|     |         | 1: Mount EMMC-GPP as read-only     |
|     |         | 2: Mount EMMC-GPP as read/write    |
| 4:5 | 0       | 0: Don't mount EMMC-BOOT0          |
|     |         | 1: Mount EMMC-BOOT0 as read-only   |
|     |         | 2: Mount EMMC-BOOT0 as read/write  |
| 6:7 | 0       | 0: Don't mount EMMC-BOOT1          |
|     |         | 1: Mount SD EMMC-BOOT1 read-only   |
|     |         | 2: Mount SD EMMC-BOOT1 read/write  |
  
Based on Hekate BDK (https://github.com/CTCaer/hekate/tree/master/bdk)
