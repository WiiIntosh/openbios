\
\ Fcode payload for Wii U GX2 graphics adapter
\
\ This is the Forth source for an Fcode payload to initialise
\ the Wii U GX2 graphics card.
\
\ (C) Copyright 2025 John Davis
\

fcode-version3

h# 500 constant cafe-video-width
h# 2D0 constant cafe-video-height
h# 20 constant cafe-depth-bits
h# 1400 constant cafe-line-bytes

: (find-xt)   \ ( str len -- xt | -1 )
  $find if
    exit
  else
    2drop
    -1
  then
;

" openbios-video-width" (find-xt) cell+ value openbios-video-width-xt
" openbios-video-height" (find-xt) cell+ value openbios-video-height-xt
" depth-bits" (find-xt) cell+ value depth-bits-xt
" line-bytes" (find-xt) cell+ value line-bytes-xt

\
\ Registers
\

h# c200000 constant gx2-mmio-base
h# 80000 constant gx2-mmio-length

\ Default location in MEM2 of framebuffer memory (1280x720x4)
h# 17500000 constant gx2-tv-fb-base
h# 384000 constant gx2-tv-fb-length

: gx2-mmio! ( data reg# -- )
  gx2-mmio-base + l!
;

external

: color!  ( r g b c# -- )
  \ TODO: Not implementing this function results in a stack overflow
  \ due to broken code, for now just pop the 4 from the stack.
  2drop
  2drop
;

headerless

\
\ Installation
\

" display" device-type
gx2-mmio-base encode-int gx2-mmio-length encode-int encode+ " reg" property

: wii-gx2-driver-install ( -- )
  \ Configure D1GRPH_SWAP_CNTL for full big endian mode.
  \ By default A and B are correct, but R and G are swapped.
  h# 2 h# 610C gx2-mmio!

  gx2-tv-fb-base to frame-buffer-adr
  default-font set-font

  frame-buffer-adr encode-int " address" property

  cafe-video-width cafe-video-height over char-width / over char-height /
  fb8-install
;

: wii-gx2-driver-init
  cafe-video-height openbios-video-height-xt !
  cafe-video-width openbios-video-width-xt !
  cafe-depth-bits depth-bits-xt !
  cafe-line-bytes line-bytes-xt !

  cafe-video-height encode-int " height" property
  cafe-video-width encode-int " width" property
  cafe-depth-bits encode-int " depth" property
  cafe-line-bytes encode-int " linebytes" property

  ['] wii-gx2-driver-install is-install
;

wii-gx2-driver-init

end0
