\
\ Fcode payload for "Flipper" video interface
\
\ This is the Forth source for an Fcode payload to initialise
\ the "Flipper" video interface.
\
\ (C) Copyright 2025 John Davis
\

fcode-version3

d# 640 constant flipper-vi-video-width
d# 480 constant flipper-vi-video-height

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

" fb8-fillrect" (find-xt) value fb8-fillrect-xt
: fb8-fillrect fb8-fillrect-xt execute ;

external

: color!  ( r g b c# -- )
  \ TODO: Not implementing this function results in a stack overflow
  \ due to broken code, for now just pop the 4 from the stack.
  2drop
  2drop
;

: fill-rectangle  ( color_ind x y width height -- )
  fb8-fillrect
;

: dimensions  ( -- width height )
  flipper-vi-video-width
  flipper-vi-video-height
;

headerless

\
\ Installation
\

" display" device-type

: flipper-vi-driver-install ( -- )
  default-font set-font
  frame-buffer-adr encode-int " address" property

  flipper-vi-video-width flipper-vi-video-height over char-width / over char-height /
  fb8-install
;

: flipper-vi-driver-init
  \ Set the dictionary properties for the video system.
  flipper-vi-video-height openbios-video-height-xt !
  flipper-vi-video-width openbios-video-width-xt !

  \ Device-level properties.
  \ Address, depth, and stride are set by C code.
  flipper-vi-video-height encode-int " height" property
  flipper-vi-video-width encode-int " width" property
  depth-bits-xt @ encode-int " depth" property
  line-bytes-xt @ encode-int " linebytes" property

  ['] flipper-vi-driver-install is-install
;

flipper-vi-driver-init

end0
