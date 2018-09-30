;By Julien Magnin a.k.a MSDOS1999

;==============================================================
; WLA-DX banking setup
;==============================================================
.memorymap
defaultslot 0
slotsize $8000
slot 0 $0000
.endme

.rombankmap
bankstotal 1
banksize $8000
banks 1
.endro

;==============================================================
; SMS defines
;==============================================================


;==============================================================
; SMS rom header
;==============================================================

.bank 0 slot 0
.org $0000
;==============================================================
; Boot section
;==============================================================
    di              ; disable interrupts
    im 1            ; Interrupt mode 1
    jp main         ; jump to main program

.org $0066
;==============================================================
; Pause button handler
;==============================================================
    ; Do nothing
    retn

;==============================================================
; Main program
;==============================================================
main:
    
	ld a, $c0
	ld b, $c0
	ld c, $18
	ld d, $c0
	ld e, $78
	ld h, $c0
	ld l, $76
	
	nop
	ld bc, $2402
	ld (bc), a
	inc bc
	inc b
	dec b
	ld b, $56
	rlca
	ex af, af'
	add hl, bc
	ld a, (bc)
	dec bc
	inc c
	dec c
	ld c, $1
	rrca
	;djnz *
	ld de, $24A3
	ld (de), a
	inc de
	inc d
	dec d
	ld d, $73
	rla
	;jr *
	add hl, de
	ld a, (de)
	dec de
	inc e
	dec e
	ld e, $C8
	rra
	;jrnz, *
	ld hl, $4785
	ld ($C001), hl
	inc hl
	inc h
	dec h
	ld h, $32
	daa
	;jr z, *
	add hl, hl
	ld hl, ($C001)
	dec hl
	inc l
	dec l
	ld l, $33
	cpl
	;jr nc, *
	ld sp, $FFF8
	ld ($C002), a
	inc sp
	inc (hl)
	dec (hl)
	ld (hl), $32
	scf
	;jr c, *
	add hl, sp
	ld a, ($C002)
	dec sp
	inc a
	dec a
	ld a, $C8
	ccf ;c7
	ld b, b
	ld b, c
	ld b, d
	ld b, e
	ld b, h
	ld b, l
	ld b, (hl)
	ld b, a
	ld c, b
	ld c, c
	ld c, d
	ld c, e
	ld c, h
	ld c, l
	ld c, (hl)
	ld c, a
	ld d, b
	ld d, c
	ld d, d
	ld d, e
	ld d, h
	ld d, l
	ld d, (hl)
	ld d, a
	ld e, b
	ld e, c
	ld e, d
	ld e, e
	ld e, h
	ld e, l
	ld e, (hl)
	ld e, a
	ld h, b
	ld h, c
	ld h, d
	ld h, e
	ld h, h
	ld h, l
	ld h, (hl)
	ld h, a
	ld l, b
	ld l, c
	ld l, d
	ld l, e
	ld l, h
	ld l, l
	ld l, (hl)
	ld l, a
	ld (hl), b
	ld (hl), c
	ld (hl), d
	ld (hl), e
	ld (hl), h
	ld (hl), l
	ld (hl), a
	ld a, b
	ld a, c
	ld a, d
	ld a, e
	ld a, h
	ld a, l
	ld a, (hl)
	;halt
	ld a, a
	add a, b
	add a, c
	add a, d
	add a, e
	add a, h
	add a, l
	add a, (hl)
	add a, a
	adc a, b
	adc a, c
	adc a, d
	adc a, e
	adc a, h
	adc a, l
	adc a, (hl)
	adc a, a
	sub b
	sub c
	sub d
	sub e
	sub h
	sub l
	sub (hl)
	sub a
	sbc a, b
	sbc a, c
	sbc a, d
	sbc a, e
	sbc a, h
	sbc a, l
	sbc a, (hl)
	sbc a, a
	and b
	and c
	and d
	and e
	and h
	and l
	and (hl)
	and a
	xor b
	xor c
	xor d
	xor e
	xor h
	xor l
	xor (hl)
	xor a
	or b
	or c
	or d
	or e
	or h
	or l
	or (hl)
	or a
	cp b
	cp c
	cp d
	cp e
	cp h
	cp l
	cp (hl)
	cp a
	;ret nz
	;pop bc
	;jpnz, **
	;jp **
	;call nz, **
	;push bc
	add a, $D0
	;rst $00
	;ret z
	;ret
	;jp z, **
	;BITS
	;call z, **
	;call **
	adc a, $D0
	;rst $08
	;ret nc
	;pop de
	;jp nc, **
	;out (*), a
	;call nc, **
	;push de
	sub $92
	;rst $10
	;ret c
	exx
	;jp c, **
	;in a, (*)
	;call c, **
	;IX
	sbc a, $40
	;rst $18
	;ret po
	;pop hl
	; jp po, **
	ex (sp), hl
	;call po, **
	;push hl
	and $67
	;rst $20
	;ret pe
	;jp (hl)
	;jp pe, **
	ex de, hl
	;call pe, **
	;EXTD
	xor $67
	;rst $28
	;ret p
	;pop af
	;jp p, **
	;di
	;call p, **
	;push af
	or $67
	;rst $30
	;ret m
	ld sp, hl
	;jp m, **
	;ei
	;call m, **
	;IY
	cp $67
	;rst $38

















    ; Infinite loop to stop program
-:  jr -

