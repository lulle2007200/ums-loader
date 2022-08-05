Payload to mount SD and/or EMMC via USB.
Menu should be pretty self explanatory, allows to mount SD, EMMC-GPP, EMMC-BOOT0 and EMMC-BOOT0 in read-only or read/write mode.

The payload only uses IRAM and does not depend on files on the SD card (like e.g. hekate) so this is especially useful when your switch a) has broken RAM and you want to backup the EMMC or b) you don't have hekate nyx on your SD card but want to mount it (without removing it from your switch).

Payload can be configured by writing a configuration to the following offsets:

Offset: 0xa4
+-----+---------+------------------------------------+
| Bit | Default | Function                           |
+-----+---------+------------------------------------+
| 0   | 0       | 0: Show the interactive menu       |
|     |         | 1: Start UMS automatically and use |
|     |         |    the configuration in 0xa5       |
+-----+---------+------------------------------------+
| 1:2 | 0       | 0: Return to menu after UMS has    |
|     |         |    stopped                         |
|     |         | 1: Power off after UMS has stopped |
|     |         | 2: Reboot to RCM after UMS has     |
|     |         |    stopped                         |
+-----+---------+------------------------------------+

Offset: 0xa5
+-----+---------+------------------------------------+
| Bit | Default | Function                           |
+-----+---------+------------------------------------+
| 0:1 | 2       | 0: Don't mount SD                  |
|     |         | 1: Mount SD as read-only           |
|     |         | 2: Mount SD as read/write          |
+-----+---------+------------------------------------+
| 2:3 | 0       | 0: Don't mount EMMC-GPP            |
|     |         | 1: Mount EMMC-GPP as read-only     |
|     |         | 2: Mount EMMC-GPP as read/write    |
+-----+---------+------------------------------------+
| 4:5 | 0       | 0: Don't mount EMMC-BOOT0          |
|     |         | 1: Mount EMMC-BOOT0 as read-only   |
|     |         | 2: Mount EMMC-BOOT0 as read/write  |
+-----+---------+------------------------------------+
| 6:7 | 0       | 0: Don't mount EMMC-BOOT1          |
|     |         | 1: Mount SD EMMC-BOOT1 read-only   |
|     |         | 2: Mount SD EMMC-BOOT1 read/write  |
+-----+---------+------------------------------------+

Based on Hekate BDK (https://github.com/CTCaer/hekate/tree/master/bdk)
