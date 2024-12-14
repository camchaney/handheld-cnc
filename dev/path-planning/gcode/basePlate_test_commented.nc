%
:O1001
(T2 D=3.175 CR=0 - ZMIN=-7.366 - flat end mill)       ; Tool 2 selected, 3.175mm diameter flat end mill, Z minimum depth is -7.366mm
N10 G90                                                ; Absolute positioning mode
N11 G17                                                ; Select XY plane
N12 G21                                                ; Set units to millimeters
N13 G28 G91 Z0                                         ; Move the tool to machine home in the Z-axis (machine coordinates)
N14 G90                                                ; Return to absolute positioning mode
(2D Contour1)                                          ; Start of 2D contour operation
N15 T2                                                 ; Select Tool 2
N16 S5000 M3                                           ; Spindle on, clockwise rotation, speed 5000 RPM

N17 G17 G90                                            ; Confirm XY plane selection and absolute positioning mode
N18 G0 X213.776 Y65.699                                ; Rapid move to X213.776 Y65.699
N19 Z15.24                                             ; Move tool to Z15.24 above the part

N20 G0 Z5.08                                           ; Rapid move to Z5.08 above the part
N21 G1 Z0.635 F338.7                                   ; Linear move down to Z0.635 at a feed rate of 338.7 mm/min

N22 Z-7.049                                            ; Linear move down to Z-7.049 (cutting depth)
N23 G18 G2 X214.094 Z-7.366 I214.094 K-7.049 F1016     ; Arc move in the ZX plane to X214.094 Z-7.366, arc center at I214.094 K-7.049, feed rate 1016 mm/min
N24 G1 X214.411                                        ; Linear move to X214.411
N25 G17 G3 X214.729 Y66.016 I214.411 J66.016           ; Arc move in the XY plane to X214.729 Y66.016, arc center at I214.411 J66.016
N26 G1 Y78.016                                         ; Linear move to Y78.016
N27 G3 X208.016 Y84.729 I208.016 J78.016               ; Arc move to X208.016 Y84.729, arc center at I208.016 J78.016
N28 G1 X174.016                                        ; Linear move to X174.016
N29 G3 X167.303 Y78.016 I174.016 J78.016               ; Arc move to X167.303 Y78.016, arc center at I174.016 J78.016
N30 G1 Y54.016                                         ; Linear move to Y54.016
N31 G3 X174.016 Y47.304 I174.016 J54.016               ; Arc move to X174.016 Y47.304, arc center at I174.016 J54.016
N32 G1 X208.016                                        ; Linear move to X208.016
N33 G3 X214.729 Y54.016 I208.016 J54.016               ; Arc move to X214.729 Y54.016, arc center at I208.016 J54.016
N34 G1 Y66.016                                         ; Linear move to Y66.016
N35 G3 X214.411 Y66.334 I214.411 J66.016               ; Arc move to X214.411 Y66.334, arc center at I214.411 J66.016
N36 G1 X214.094                                        ; Linear move to X214.094
N37 G18 G3 X213.776 Z-7.049 I214.094 K-7.049           ; Arc move in the ZX plane to X213.776 Z-7.049, arc center at I214.094 K-7.049
N38 G0 Z5.08                                           ; Rapid move to Z5.08 above the part
N39 X353.333 Y-1.524                                   ; Rapid move to X353.333 Y-1.524
N40 G1 Z0.635 F338.7                                   ; Linear move down to Z0.635 at a feed rate of 338.7 mm/min

N41 Z-7.049                                            ; Linear move down to Z-7.049 (cutting depth)
N42 G19 G3 Y-1.207 Z-7.366 J-1.207 K-7.049 F1016       ; Arc move in the YZ plane to Y-1.207 Z-7.366, arc center at J-1.207 K-7.049, feed rate 1016 mm/min
N43 G1 Y-0.889                                         ; Linear move to Y-0.889
N44 G17 G3 X353.016 Y-0.572 I353.016 J-0.889           ; Arc move in the XY plane to X353.016 Y-0.572, arc center at I353.016 J-0.889
N45 G1 X9.016                                          ; Linear move to X9.016
N46 G2 X-0.572 Y9.016 I9.016 J9.016                    ; Arc move to X-0.572 Y9.016, arc center at I9.016 J9.016
N47 G1 Y243.016                                        ; Linear move to Y243.016
N48 G2 X9.016 Y252.604 I9.016 J243.016                 ; Arc move to X9.016 Y252.604, arc center at I9.016 J243.016
N49 G1 X353.016                                        ; Linear move to X353.016
N50 G2 X362.603 Y243.016 I353.016 J243.016             ; Arc move to X362.603 Y243.016, arc center at I353.016 J243.016
N51 G1 Y9.016                                          ; Linear move to Y9.016
N52 G2 X353.016 Y-0.572 I353.016 J9.016                ; Arc move to X353.016 Y-0.572, arc center at I353.016 J9.016
N53 G3 X352.698 Y-0.889 I353.016 J-0.889               ; Arc move in the XY plane to X352.698 Y-0.889, arc center at I353.016 J-0.889
N54 G1 Y-1.207                                         ; Linear move to Y-1.207
N55 G19 G2 Y-1.524 Z-7.049 J-1.207 K-7.049             ; Arc move in the YZ plane to Y-1.524 Z-7.049, arc center at J-1.207 K-7.049

N56 G0 Z5.08                                           ; Rapid move to Z5.08 above the part
N57 X213.776 Y185.699                                  ; Rapid move to X213.776 Y185.699

N58 G1 Z0.635 F338.7                                   ; Linear move down to Z0.635 at a feed rate of 338.7 mm/min
N59 Z-7.049                                            ; Linear move down to Z-7.049 (cutting depth)
N60 G18 G2 X214.094 Z-7.366 I214.094 K-7.049 F1016     ; Arc move in the ZX plane to X214.094 Z-7.366, arc center at I214.094 K-7.049, feed rate 1016 mm/min
N61 G1 X214.411                                        ; Linear move to X214.411
N62 G17 G3 X214.729 Y186.016 I214.411 J186.016         ; Arc move in the XY plane to X214.729 Y186.016, arc center at I214.411 J186.016
N63 G1 Y198.016                                        ; Linear move to Y198.016
N64 G3 X208.016 Y204.729 I208.016 J198.016             ; Arc move to X208.016 Y204.729, arc center at I208.016 J198.016
N65 G1 X174.016                                        ; Linear move to X174.016
N66 G3 X167.303 Y198.016 I174.016 J198.016             ; Arc move to X167.303 Y198.016, arc center at I174.016 J198.016
N67 G1 Y174.016                                        ; Linear move to Y174.016
N68 G3 X174.016 Y167.303 I174.016 J174.016             ; Arc move to X174.016 Y167.303, arc center at I174.016 J174.016
N69 G1 X208.016                                        ; Linear move to X208.016
N70 G3 X214.729 Y174.016 I208.016 J174.016             ; Arc move to X214.729 Y174.016, arc center at I208.016 J174.016
N71 G1 Y186.016                                        ; Linear move to Y186.016
N72 G3 X214.411 Y186.333 I214.411 J186.016             ; Arc
