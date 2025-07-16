\   Wii specific initialization code
\
\   Copyright (C) 2025 John Davis
\
\   This program is free software; you can redistribute it and/or
\   modify it under the terms of the GNU General Public License
\   as published by the Free Software Foundation
\

include config.fs

\ ---------
\ DMA words
\ ---------

: ppc-dma-free  ( virt size -- )
  2drop
;

: ppc-dma-map-out  ( virt devaddr size -- )
  (dma-sync)
;

['] ppc-dma-free to (dma-free)
['] ppc-dma-map-out to (dma-map-out)

\ -------------------------------------------------------------
\ device-tree
\ -------------------------------------------------------------

" /" find-device
\ Apple calls the root node device-tree
" device-tree" device-name
" bootrom" device-type
" 0000000000000" encode-string " system-id" property
1 encode-int " #address-cells" property
1 encode-int " #size-cells" property
1 encode-int " #interrupt-cells" property

    : dma-sync
      (dma-sync)
    ;

    : dma-alloc
      (dma-alloc)
    ;

    : dma-free
      (dma-free)
    ;

    : dma-map-in
      (dma-map-in)
    ;

    : dma-map-out
      (dma-map-out)
    ;

new-device
    " cpus" device-name
    1 encode-int " #address-cells" property
    0 encode-int " #size-cells" property
    external

    : encode-unit ( unit -- str len )
        pocket tohexstr
    ;

    : decode-unit ( str len -- unit )
        parse-hex
    ;

finish-device

new-device
    " memory" device-name
    " memory" device-type
    external
    : open true ;
    : close ;
finish-device

\ Required for Mac OS X. TODO: investigate to see if this interferes with OS8 or OS9, or if there is a better way.
\ Taken from MOL.
new-device
    " mac-io" device-name
    " mac-io" device-type
    " AAPL,Keylargo-DUMMY" model
    h# 8200b810 encode-int 00000000 encode-int encode+ 80000000 encode-int encode+ 00000000 encode-int encode+ 00080000 encode-int encode+ " assigned-addresses" property
finish-device

\ -------------------------------------------------------------
\ Base hardware devices on both Wii and Wii U. Wii U-specific items are added in C code.
\ -------------------------------------------------------------

\ Processor interface interrupt controller
new-device
    " interrupt-controller" device-name
    " interrupt-controller" device-type
    " Wii,PIC" model
    " wii-pic" encode-string " compatible" property
    " " encode-string " built-in" property
    " " encode-string " interrupt-controller" property
    1 encode-int " #interrupt-cells" property
    \ C code to populate reg property
    external
    : open true ;
    : close ;
finish-device

\ AES engine
new-device
    " aes" device-name
    " Wii,AES" model
    " wii-aes" encode-string " compatible" property
    " " encode-string " built-in" property
    h# 0d020000 encode-int 14 encode-int encode+ " reg" property
    d# 2 encode-int " interrupts" property
    \ C code to populate interrupt-parent property
    external
    : open true ;
    : close ;
finish-device

\ SHA-1 engine
new-device
    " sha" device-name
    " Wii,SHA" model
    " wii-sha" encode-string " compatible" property
    " " encode-string " built-in" property
    h# 0d030000 encode-int 1c encode-int encode+ " reg" property
    d# 3 encode-int " interrupts" property
    \ C code to populate interrupt-parent property
    external
    : open true ;
    : close ;
finish-device

\ EHCI USB controller 0 (rear USB ports)
new-device
    " ehc0" device-name
    " usb" device-type
    " Wii,EHCI" model
    " wii-ehci" encode-string " compatible" property
    " " encode-string " built-in" property
    h# 0d040000 encode-int 100 encode-int encode+ " reg" property
    d# 4 encode-int " interrupts" property
    \ C code to populate interrupt-parent property
    external
    : open true ;
    : close ;
finish-device

\ OHCI USB controller 0:0 (rear USB ports)
new-device
    " ohc0" device-name
    " usb" device-type
    " Wii,OHCI" model
    " wii-ohci" encode-string " compatible" property
    " " encode-string " built-in" property
    h# 0d050000 encode-int 100 encode-int encode+ " reg" property
    d# 5 encode-int " interrupts" property
    \ C code to populate interrupt-parent property
    external
    : open true ;
    : close ;
finish-device

\ OHCI USB controller 0:1 (internal for Bluetooth)
new-device
    " ohc1" device-name
    " usb" device-type
    " Wii,OHCI" model
    " wii-ohci" encode-string " compatible" property
    " " encode-string " built-in" property
    h# 0d060000 encode-int 100 encode-int encode+ " reg" property
    d# 6 encode-int " interrupts" property
    \ C code to populate interrupt-parent property
    external
    : open true ;
    : close ;
finish-device

\ SDHC controller (front SD card slot)
new-device
    " sdhc" device-name
    " Wii,SDHC" model
    " wii-sdhc" encode-string " compatible" property
    " " encode-string " built-in" property
    h# 0d070000 encode-int 200 encode-int encode+ " reg" property
    d# 7 encode-int " interrupts" property
    \ C code to populate interrupt-parent property
    external
    : open true ;
    : close ;
finish-device

\ SDIO controller (internal WiFi)
new-device
    " sdio" device-name
    " Wii,SDIO" model
    " wii-sdio" encode-string " compatible" property
    " " encode-string " built-in" property
    h# 0d080000 encode-int 200 encode-int encode+ " reg" property
    d# 8 encode-int " interrupts" property
    \ C code to populate interrupt-parent property
    external
    : open true ;
    : close ;
finish-device

\ -------------------------------------------------------------
\ /packages
\ -------------------------------------------------------------

" /packages" find-device

    " packages" device-name
    external
    \ allow packages to be opened with open-dev
    : open true ;
    : close ;

\ /packages/terminal-emulator
new-device
    " terminal-emulator" device-name
    external
    : open true ;
    : close ;
    \ : write ( addr len -- actual )
    \	dup -rot type
    \ ;
finish-device

\ -------------------------------------------------------------
\ The END
\ -------------------------------------------------------------
device-end
