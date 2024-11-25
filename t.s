	.text
	.attribute	4, 16
	.attribute	5, "rv32i2p1_m2p0_zmmul1p0"
	.file	"main.c"
	.globl	factorial                       # -- Begin function factorial
	.p2align	2
	.type	factorial,@function
factorial:                              # @factorial
# %bb.0:
	lw	a1, %lo(sa)(a1)
	addi	a1, a1, %lo(sb)
	addi	a1, a1, %lo(sc)
.LBB0_1:                                # =>This Inner Loop Header: Depth=1
	j	.LBB0_2
.LBB0_2:                                #   in Loop: Header=BB0_1 Depth=1
	j	.LBB0_3
.LBB0_3:                                #   in Loop: Header=BB0_1 Depth=1
	j	.LBB0_1
.LBB0_4:
	ret
.Lfunc_end0:
	.size	factorial, .Lfunc_end0-factorial
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	2
	.type	main,@function
main:                                   # @main
# %bb.0:
	ret
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
                                        # -- End function
	.type	sa,@object                      # @sa
	.section	.sdata,"aw",@progbits
	.globl	sa
	.p2align	2, 0x0
sa:
	.word	100                             # 0x64
	.size	sa, 4

	.type	sb,@object                      # @sb
	.section	.rodata,"a",@progbits
	.p2align	2, 0x0
sb:
	.word	20                              # 0x14
	.size	sb, 4

	.type	sc,@object                      # @sc
  .section	.rodata,"a",@progbits
sc:
	.asciz	"gfhsdfg\000\000\000\000\000\000\000\000\000\000\000\000"
	.size	sc, 20

	.type	.L__const.main.a,@object        # @__const.main.a
	.section	.rodata,"a",@progbits
.L__const.main.a:
	.asciz	"asdfg\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
	.size	.L__const.main.a, 20

	.ident	"Homebrew clang version 19.1.3"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym factorial
	.addrsig_sym sa
	.addrsig_sym sb
	.addrsig_sym sc
