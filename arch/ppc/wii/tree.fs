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

\ ROM device required for classic Mac OS
" /" find-device
new-device
  " rom" device-name
finish-device

\ -------------------------------------------------------------
\ Base hardware devices on both Wii and Wii U. Platform-specific items are addeded later.
\ -------------------------------------------------------------

\ Processor interface interrupt controller
new-device
  " interrupt-controller" device-name
  " interrupt-controller" device-type
  " NTDOY,pic" model
  " NTDOY,pic" encode-string " compatible" property
  " " encode-string " built-in" property
  " " encode-string " interrupt-controller" property
  1 encode-int " #interrupt-cells" property
  \ Fcode to populate reg property
  external
  : open true ;
  : close ;
finish-device

\ Serial interface
new-device
  " si" device-name
  " NTDOY,si" model
  " NTDOY,si" encode-string " compatible" property
  " " encode-string " built-in" property
  h# 0d006400 encode-int 100 encode-int encode+ " reg" property
  d# 3 encode-int " interrupts" property
  " /interrupt-controller" find-dev if
    encode-int " interrupt-parent" property
  then
  external
  : open true ;
  : close ;
finish-device

\ External interface
new-device
  " exi" device-name
  " NTDOY,exi" model
  " NTDOY,exi" encode-string " compatible" property
  " " encode-string " built-in" property
  h# 0d006800 encode-int 40 encode-int encode+ " reg" property
  d# 4 encode-int " interrupts" property
  " /interrupt-controller" find-dev if
    encode-int " interrupt-parent" property
  then
  external
  : open true ;
  : close ;
finish-device

\ Audio interface
new-device
  " audio-controller" device-name
  " NTDOY,audio" model
  " NTDOY,audio" encode-string " compatible" property
  " " encode-string " built-in" property
  h# 0d006c00 encode-int 20 encode-int encode+ " reg" property
  d# 5 encode-int " interrupts" property
  " /interrupt-controller" find-dev if
    encode-int " interrupt-parent" property
  then
  external
  : open true ;
  : close ;
finish-device

\ DSP
new-device
  " dsp" device-name
  " NTDOY,dsp" model
  " NTDOY,dsp" encode-string " compatible" property
  " " encode-string " built-in" property
  \ Fcode to populate reg property
  d# 6 encode-int " interrupts" property
  " /interrupt-controller" find-dev if
    encode-int " interrupt-parent" property
  then
  external
  : open true ;
  : close ;
finish-device

\ AES engine
new-device
  " aes" device-name
  " NTDOY,aes" model
  " NTDOY,aes" encode-string " compatible" property
  " " encode-string " built-in" property
  h# 0d020000 encode-int 14 encode-int encode+ " reg" property
  d# 2 encode-int " interrupts" property
  \ Fcode to populate interrupt-parent property
  external
  : open true ;
  : close ;
finish-device

\ SHA-1 engine
new-device
  " sha" device-name
  " NTDOY,sha" model
  " NTDOY,sha" encode-string " compatible" property
  " " encode-string " built-in" property
  h# 0d030000 encode-int 1c encode-int encode+ " reg" property
  d# 3 encode-int " interrupts" property
  \ Fcode to populate interrupt-parent property
  external
  : open true ;
  : close ;
finish-device

\ EHCI USB controller 0 (rear USB ports)
new-device
  " usb" device-name
  " usb" device-type
  " NTDOY,ehci" model
  " NTDOY,ehci" encode-string " compatible" property
  " " encode-string " built-in" property
  h# 0d040000 encode-int 100 encode-int encode+ " reg" property
  d# 4 encode-int " interrupts" property
  \ Fcode to populate interrupt-parent property
  external
  : open true ;
  : close ;
finish-device

\ OHCI USB controller 0:0 (rear USB ports)
new-device
  " usb" device-name
  " usb" device-type
  " NTDOY,ohci" model
  " NTDOY,ohci" encode-string " compatible" property
  " " encode-string " built-in" property
  h# 0d050000 encode-int 100 encode-int encode+ " reg" property
  d# 5 encode-int " interrupts" property
  \ Fcode to populate interrupt-parent property
  external
  : open true ;
  : close ;
finish-device

\ OHCI USB controller 0:1 (internal for Bluetooth)
new-device
  " usb" device-name
  " usb" device-type
  " NTDOY,ohci" model
  " NTDOY,ohci" encode-string " compatible" property
  " " encode-string " built-in" property
  h# 0d060000 encode-int 100 encode-int encode+ " reg" property
  d# 6 encode-int " interrupts" property
  \ Fcode to populate interrupt-parent property
  external
  : open true ;
  : close ;
finish-device

\ SDHC controller (front SD card slot)
new-device
  " sdhc" device-name
  " NTDOY,sdhc" model
  " NTDOY,sdhc" encode-string " compatible" property
  " " encode-string " built-in" property
  h# 0d070000 encode-int 200 encode-int encode+ " reg" property
  d# 7 encode-int " interrupts" property
  \ Fcode to populate interrupt-parent property
  external
  : open true ;
  : close ;
finish-device

\ SDIO controller (internal WiFi)
new-device
  " sdio" device-name
  " NTDOY,sdio" model
  " NTDOY,sdio" encode-string " compatible" property
  " " encode-string " built-in" property
  h# 0d080000 encode-int 200 encode-int encode+ " reg" property
  d# 8 encode-int " interrupts" property
  \ Fcode to populate interrupt-parent property
  external
  : open true ;
  : close ;
finish-device

\ IPC
" /" find-device
new-device
  " ipc" device-name
  " NTDOY,ipc" model
  " NTDOY,ipc" encode-string " compatible" property
  " " encode-string " built-in" property
  h# 0d800000 encode-int 10 encode-int encode+ " reg" property
  d# 30 encode-int " interrupts" property
  \ Fcode to populate interrupt-parent property
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
