\   Wii specific initialization code
\
\   Copyright (C) 2005 Stefan Reinauer
\   Copyright (C) 2025 John Davis
\
\   This program is free software; you can redistribute it and/or
\   modify it under the terms of the GNU General Public License
\   as published by the Free Software Foundation
\


\ -------------------------------------------------------------------------
\ initialization
\ -------------------------------------------------------------------------

: make-openable ( path )
  find-dev if
    begin ?dup while
      \ install trivial open and close methods
      dup active-package! is-open
      parent
    repeat
  then
;

: preopen ( chosen-str node-path )
  2dup make-openable

  " /chosen" find-device
  open-dev ?dup if
    encode-int 2swap property
  else
    2drop
  then
;

\ preopen device nodes (and store the ihandles under /chosen)
:noname
  " rtc" " rtc" preopen
  " memory" " /memory" preopen
; SYSTEM-initializer


\ use the tty interface if available
: activate-tty-interface
  " /packages/terminal-emulator" find-dev if drop
  then
;

variable keyboard-phandle 0 keyboard-phandle !

: (find-keyboard-device) ( phandle -- )
  recursive
  keyboard-phandle @ 0= if  \ Return first match
    >dn.child @
    begin ?dup while
      dup dup " device_type" rot get-package-property 0= if
        drop dup cstrlen
        " keyboard" strcmp 0= if
          dup to keyboard-phandle
        then
      then
      (find-keyboard-device)
      >dn.peer @
    repeat
  else
    drop
  then
;

\ create the keyboard devalias
:noname
  device-tree @ (find-keyboard-device)
  keyboard-phandle @ if
    active-package
    " /aliases" find-device
    keyboard-phandle @ get-package-path 2dup
    encode-string " kbd" property
    encode-string " keyboard" property
    active-package!
  then
; SYSTEM-initializer

\ Fixup device tree for Wii platform
: fixup-device-tree-rvl ( -- )
  active-package

  \ DT root
  " /" find-device
  " Wii" model
  " Wii" encode-string " NTDOY,Revolution" encode-string encode+ " compatible" property \ TODO: System Profiler takes the first string in compatible
  h# e7be2c0 encode-int " clock-frequency" property

  \ Platform interrupt controller MMIO
  " /interrupt-controller" find-device
  h# 0c003000 encode-int 8 encode-int encode+ " reg" property

  \ Create Hollywood interrupt controller
  " /" find-device
  new-device
    " interrupt-controller" device-name
    " interrupt-controller" device-type
    " NTDOY,hlwd-pic" model
    " NTDOY,hlwd-pic" encode-string " compatible" property
    " " encode-string " built-in" property
    " " encode-string " interrupt-controller" property
    1 encode-int " #interrupt-cells" property
    h# 0d000030 encode-int 10 encode-int encode+ " reg" property
    d# 14 encode-int " interrupts" property
    " /interrupt-controller@0c003000" find-dev if
      encode-int " interrupt-parent" property
    then
  finish-device

  \ Set Hollywood as interrupt controller parent on Wii hardware devices
  " /aes" find-device
  " /interrupt-controller@0d000030" find-dev if
    encode-int " interrupt-parent" property
  then
  " /sha" find-device
  " /interrupt-controller@0d000030" find-dev if
    encode-int " interrupt-parent" property
  then
  " /usb@0d040000" find-device
  " /interrupt-controller@0d000030" find-dev if
    encode-int " interrupt-parent" property
  then
  " /usb@0d050000" find-device
  " /interrupt-controller@0d000030" find-dev if
    encode-int " interrupt-parent" property
  then
  " /usb@0d060000" find-device
  " /interrupt-controller@0d000030" find-dev if
    encode-int " interrupt-parent" property
  then
  " /sdhc" find-device
  " /interrupt-controller@0d000030" find-dev if
    encode-int " interrupt-parent" property
  then
  " /sdio" find-device
  " /interrupt-controller@0d000030" find-dev if
    encode-int " interrupt-parent" property
  then

  active-package!
;

\ Fixup device tree for Wii U platform
: fixup-device-tree-cafe ( -- )
  active-package

  \ DT root
  " /" find-device
  " NTDOY,Cafe" model
  " WiiU" encode-string " NTDOY,Cafe" encode-string encode+ " compatible" property \ TODO: System Profiler takes the first string in compatible
  h# ed1b768 encode-int " clock-frequency" property

  \ Platform interrupt controller MMIO
  " /interrupt-controller" find-device
  h# 0c000000 encode-int 100 encode-int encode+ " reg" property

  \ Create Latte interrupt controller
  " /" find-device
  new-device
    " interrupt-controller" device-name
    " interrupt-controller" device-type
    " NTDOY,latte-pic" model
    " NTDOY,latte-pic" encode-string " compatible" property
    " " encode-string " built-in" property
    " " encode-string " interrupt-controller" property
    1 encode-int " #interrupt-cells" property
    h# 0d800440 encode-int 48 encode-int encode+ " reg" property
    d# 24 encode-int " interrupts" property
    " /interrupt-controller@0c000000" find-dev if
      encode-int " interrupt-parent" property
    then
  finish-device

  \ Set Latte as interrupt controller parent on Wii hardware devices
  " /aes" find-device
  " /interrupt-controller@0d800440" find-dev if
    encode-int " interrupt-parent" property
  then
  " /sha" find-device
  " /interrupt-controller@0d800440" find-dev if
    encode-int " interrupt-parent" property
  then
  " /usb@0d040000" find-device
  " /interrupt-controller@0d800440" find-dev if
    encode-int " interrupt-parent" property
  then
  " /usb@0d050000" find-device
  " /interrupt-controller@0d800440" find-dev if
    encode-int " interrupt-parent" property
  then
  " /usb@0d060000" find-device
  " /interrupt-controller@0d800440" find-dev if
    encode-int " interrupt-parent" property
  then
  " /sdhc" find-device
  " /interrupt-controller@0d800440" find-dev if
    encode-int " interrupt-parent" property
  then
  " /sdio" find-device
  " /interrupt-controller@0d800440" find-dev if
    encode-int " interrupt-parent" property
  then

  \ Create GX2 GPU stub device
  \ Additional properties will be added in the GX2 driver.
  " /" find-device
  new-device
    " gx2" device-name
    " NTDOY,gx2" model
    " NTDOY,gx2" encode-string " compatible" property
    " " encode-string " built-in" property
    d# 2 encode-int " interrupts" property
    " /interrupt-controller@0c000000" find-dev if
      encode-int " interrupt-parent" property
    then
  finish-device

  active-package!
;

\ -------------------------------------------------------------------------
\ pre-booting
\ -------------------------------------------------------------------------

: update-chosen
  " /chosen" find-device
  stdin @ encode-int " stdin" property
  stdout @ encode-int " stdout" property
  device-end
;

:noname
  set-defaults
; PREPOST-initializer

\ -------------------------------------------------------------------------
\ Mac OF specific words
\ -------------------------------------------------------------------------

: parse-1hex 1 parse-nhex ;
: parse-2hex 2 parse-nhex ;
: parse-3hex 3 parse-nhex ;

\ -------------------------------------------------------------------------
\ copyright property handling
\ -------------------------------------------------------------------------

: insert-copyright-property
  \ As required for MacOS 9 and below
  " Pbclevtug 1983-2001 Nccyr Pbzchgre, Vap. GUVF ZRFFNTR SBE PBZCNGVOVYVGL BAYL"
  rot13-str encode-string " copyright"
  " /" find-package if
    " set-property" $find if
      execute
    else
      3drop drop
    then
  then
;

: delete-copyright-property
  \ Remove copyright property created above
  active-package
  " /" find-package if
      active-package!
      " copyright" delete-property
  then
  active-package!
;

: (exit)
  \ Clean up before returning to the interpreter
  delete-copyright-property
;

\ -------------------------------------------------------------------------
\ Adler-32 wrapper
\ -------------------------------------------------------------------------

: adler32 ( adler buf len -- checksum )
  " (adler32)" $find if
    execute
  else
    ." Can't find " ( adler32-name ) type cr
    3drop 0
  then
;

[IFDEF] CONFIG_DRIVER_WII_GX2
  -1 value wii-gx2-driver-fcode
  " Wii,GX2.bin" $encode-file to wii-gx2-driver-fcode
[THEN]
