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

external

: color!  ( r g b c# -- )
  \ TODO: Not implementing this function results in a stack overflow
  \ due to broken code, for now just pop the 4 from the stack.
  2drop
  2drop
;

\
\ Installation
\

: wii-gx2-driver-install ( -- )
  h# 17500000 to frame-buffer-adr
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
