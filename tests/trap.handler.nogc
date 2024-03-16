# SPIM S20 MIPS simulator.
#
# Modified trap handler for COOL runtime.
#
# 8/19/94 Manuel Fahndrich
#
# $Log: trap.handler.nogc,v $
# Revision 1.1.1.1  1996/07/12 06:46:20  aiken
# Imported sources
#
# Revision 1.13  1995/08/26 11:42:50  aiken
# updated for 1995 version of Cool
#
# Revision 1.12  1994/11/15 03:33:34  manuel
# substr method didn't allow taking the empty substr at the end of a
# string
#
#   Revision 1.11  1994/11/14  21:41:23  manuel
#   Comment for equality_test contained a type: arguments are in $t1 and
#   $t2
#
#   Revision 1.10  1994/10/26  02:31:50  manuel
#   Added more comments.
#
#   Revision 1.9  1994/08/31  02:04:31  manuel
#   Fixed an error in the in_string code: Reading from EOF, the system
#   returns 0 characters. We test for this and return a single '\n'. The
#   code can therefore recognize EOF.
#   The last line of a file must be terminated by a newline, otherwise
#   spim gets confused and returns the entire buffer!
#
#   Revision 1.8  1994/08/28  02:21:46  manuel
#   - Fixed typo in system message
#
#   Revision 1.7  1994/08/27  08:53:35  manuel
#   - Added an .align at end of data segment to be safe. Cgen should emit
#     one.
#
#   Revision 1.6  1994/08/27  08:37:49  manuel
#   - Added string primitives
#
#   Revision 1.5  1994/08/27  04:42:25  manuel
#   - Adapted code to handle String Class containing an Int object slot
#     for the string size.
#
#   Revision 1.4  1994/08/27  02:01:43  manuel
#   - Fixed typos
#
#   Revision 1.2  1994/08/27  00:41:01  manuel
#   - Changed string object representation to two slots. The first is a 32
#     bit slot indicating the string length, the second is a variable
#     sized slot containing the actual null terminated string.
#   - Fixed a bug in the in_string function which set obj_size to
#     4*obj_size
#   - Added constants for field offsets
#
#
# SPIM is distributed under the following conditions:
#
# You may make copies of SPIM for your own use and modify those copies.
#
# All copies of SPIM must retain my name and copyright notice.
#
# You may not sell SPIM or distributed SPIM in conjunction with a commerical
# product or service without the expressed written consent of James Larus.
#
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE.
#
# Define the exception handling code.  This must go first!

	.kdata
__m1_:	.asciiz "  Exception "
__m2_:	.asciiz " Execution aborted\n"
__e0_:	.asciiz "  [Interrupt] "
__e1_:	.asciiz	""
__e2_:	.asciiz	""
__e3_:	.asciiz	""
__e4_:	.asciiz	"  [Unaligned address in inst/data fetch] "
__e5_:	.asciiz	"  [Unaligned address in store] "
__e6_:	.asciiz	"  [Bad address in text read] "
__e7_:	.asciiz	"  [Bad address in data/stack read] "
__e8_:	.asciiz	"  [Error in syscall] "
__e9_:	.asciiz	"  [Breakpoint/Division by 0] "
__e10_:	.asciiz	"  [Reserved instruction] "
__e11_:	.asciiz	""
__e12_:	.asciiz	"  [Arithmetic overflow] "
__e13_:	.asciiz	"  [Inexact floating point result] "
__e14_:	.asciiz	"  [Invalid floating point result] "
__e15_:	.asciiz	"  [Divide by 0] "
__e16_:	.asciiz	"  [Floating point overflow] "
__e17_:	.asciiz	"  [Floating point underflow] "
__excp:	.word __e0_,__e1_,__e2_,__e3_,__e4_,__e5_,__e6_,__e7_,__e8_,__e9_
	.word __e10_,__e11_,__e12_,__e13_,__e14_,__e15_,__e16_,__e17_
s1:	.word 0
s2:	.word 0

	.ktext 0x80000080
	.set noat
	# Because we are running in the kernel, we can use $k0/$k1 without
	# saving their old values.
	move $at $k1	# Save $at
	.set at
	sw $v0 s1	# Not re-entrent and we can't trust $sp
	sw $a0 s2
	mfc0 $k0 $13	# Cause
        sgt $v0 $k0 0x44 # ignore interrupt exceptions
        bgtz $v0 ret
        addu $0 $0 0
	li $v0 4	# syscall 4 (print_str)
	la $a0 __m1_
	syscall
	li $v0 1	# syscall 1 (print_int)
        srl $a0 $k0 2	# shift Cause reg
	syscall
	li $v0 4	# syscall 4 (print_str)
	lw $a0 __excp($k0)
	syscall
	li $v0 4
	la $a0 __m2_
	syscall
	li $v0 10	# Exit upon all exceptions
	syscall		# syscall 10 (exit)
ret:	lw $v0 s1
	lw $a0 s2
	mfc0 $k0 $14	# EPC
	.set noat
	move $k1 $at	# Restore $at
	.set at
	rfe		# Return from exception handler
	addiu $k0 $k0 4 # Return to next instruction
	jr $k0

#
# Functions that return to the cool caller, should preserve $s0-$s7
#
# $s7 is reserved as the limit pointer.
# $gp is the heap pointer (points to the next unused word)
#	should never be handled by the generated code!!!
# $sp is the stack pointer
# $ra contains the return address
#
# $v0, $v1, $t0, $t1, $t2, $a0, $a1, $a2 are scratch registers
#  (i.e. caller cannot assume that they remain unchanged)
#

# Standard startup code.  Invoke the routine main with no arguments.
	.data
_abort_msg:	.asciiz "Abort called from class "
_colon_msg:	.asciiz ":"
_dispatch_msg:  .asciiz ": Dispatch to void.\n"
_cabort_msg:	.asciiz "No match in case statement for Class "
_cabort_msg2:   .asciiz ": Match on void in case statement.\n"
_nl:		.asciiz "\n"
_term_msg:	.asciiz "COOL program successfully executed\n"
_heap_msg:	.asciiz "Increasing heap...\n"
_sabort_msg1:	.asciiz	"Index to substr is negative\n"
_sabort_msg2:	.asciiz	"Index to substr is too big\n"
_sabort_msg3:	.asciiz	"Length to substr too long\n"
_sabort_msg4:	.asciiz	"Length to substr is negative\n"
_sabort_msg:	.asciiz "Execution aborted.\n"

	.align 2

# Define some constants
#
obj_tag=0
obj_size=4
disp_tab=8
int_slot=12
bool_slot=12
str_size=12	# This is a pointer to an Int object!!!
str_field=16	# The beginning of the ascii sequence

	.text
	.globl __start
__start: 
	li	$v0 9
	move	$a0 $zero
	syscall			# sbrk
	move	$s7 $v0		# init limit pointer
	la	$gp heap_start	# init heap pointer
        la      $a0 Main_protObj
        jal     Object.copy	# Call copy
        jal     Main_init
	
	jal	Main.main	# Invoke main method
	
	la	$a0 _term_msg
	li	$v0 4
	syscall
	li $v0 10
	syscall		# syscall 10 (exit)

#
#  Polymorphic equality testing function:
#  Two objects are equal if they are
#    - identical (pointer equality, inlined in code)
#    - have same tag and are of type BOOL,STRING,INT and contain the
#      same data
#
#  INPUT: The two objects are passed in $t1 and $t2
#  OUTPUT: Initial value of $a0, if the objects are equal
#          Initial value of $a1, otherwise
#
#  The tags for Int,Bool,String are found in the global locations
#  _int_tag, _bool_tag, _string_tag, which are initialized by the
#  data part of the generated code. This removes a consistency problem
#  between this file and the generated code.
#
	.globl	equality_test
equality_test:			# ops in $t1 $t2
				# true in A0, false in A1
				# assume $t1, $t2 are not equal
	beq	$t1 $zero eq_false # $t2 can't also be void   
	beq     $t2 $zero eq_false # $t1 can't also be void   
	lw	$v0 obj_tag($t1)	# get tags
	lw	$v1 obj_tag($t2)
	bne	$v1 $v0 eq_false	# compare tags
	lw	$a2 _int_tag	# load int tag
	beq	$v1 $a2 eq_int	# Integers
	lw	$a2 _bool_tag	# load bool tag
	beq	$v1 $a2 eq_int	# Booleans
	lw	$a2 _string_tag # load string tag
	bne	$v1 $a2 eq_false  # Not a primitive type
eq_str: # handle strings
	lw	$v0, str_size($t1)	# get string size objs
	lw	$v1, str_size($t2)
	lw	$v0, int_slot($v0)	# get string sizes
	lw	$v1, int_slot($v1)
	bne	$v1 $v0 eq_false
	beqz	$v1 eq_true		# 0 length strings are equal
	add	$t1 str_field		# Point to start of string
	add	$t2 str_field
	move	$t0 $v0		# Keep string length as counter
eq_l1:
	lbu	$v0,0($t1)	# get char
	add	$t1 1
	lbu	$v1,0($t2)
	add	$t2 1
	bne	$v1 $v0 eq_false
	addiu	$t0 $t0 -1	# Decrement counter
	bnez	$t0 eq_l1
	b	eq_true		# end of strings
		
eq_int:	# handles booleans and ints
	lw	$v0,int_slot($t1)	# load values
	lw	$v1,int_slot($t2)
	bne	$v1 $v0 eq_false
eq_true:
	jr	$ra		# return true
eq_false:
	move	$a0 $a1		# move false into accumulator
	jr	$ra

#
#  _dispatch_abort
#
#      filename in $a0
#      line number in $t1
#  
#  Prints error message and exits.
#  Called on dispatch to void.
#
	.globl	_dispatch_abort
_dispatch_abort:		 
        sw      $t1 0($sp)       # save line number
        addiu   $sp $sp -4
	addiu   $a0 $a0 str_field # adjust to beginning of string
	li      $v0 4
	syscall                  # print file name
	la      $a0 _colon_msg
	li	$v0 4
	syscall                  # print ":"
	lw      $a0 4($sp)       # 
	li	$v0 1
	syscall			 # print line number
	li 	$v0 4
	la	$a0 _dispatch_msg
	syscall			 # print dispatch-to-void message
	li   	$v0 10
        syscall			 # exit


#
#  _case_abort2
#
#      filename in $a0
#      line number in $t1
#  
#  Prints error message and exits.
#  Called on case on void.
#
	.globl	_case_abort2
_case_abort2:		 
        sw      $t1 0($sp)       # save line number
        addiu   $sp $sp -4
	addiu   $a0 $a0 str_field # adjust to beginning of string
	li      $v0 4
	syscall                  # print file name
	la      $a0 _colon_msg
	li	$v0 4
	syscall                  # print ":"
	lw      $a0 4($sp)       # 
	li	$v0 1
	syscall			 # print line number
	li 	$v0 4
	la	$a0 _cabort_msg2
	syscall			 # print case-on-void message
	li   	$v0 10
        syscall			 # exit
	
#
#
#  _case_abort
#		Is called when a case statement has no match
#
#   INPUT:	$a0 contains the object on which the case was
#		performed
#
#   Does not return!
#
	.globl	_case_abort
_case_abort:			# $a0 contains case expression obj.
	move	$s0 $a0		# save the expression object
	la	$a0 _cabort_msg
	li	$v0 4
	syscall			# print_str
	la	$t1 class_nameTab
	lw	$v0 obj_tag($s0)	# Get object tag
	sll	$v0 $v0 2	# *4
	addu	$t1 $t1 $v0
	lw	$t1 0($t1)	# Load class name string obj.
	addiu	$a0 $t1 str_field # Adjust to beginning of str
	li	$v0 4		# print_str
	syscall
	la	$a0 _nl
	li	$v0 4		# print_str
	syscall
	li	$v0 10
	syscall			# Exit
	

#	
#
# Copy method
#
#   INPUT:	$a0: object to be copied to free space in heap
#
#   OUTPUT:	$a0 points to the newly created copy.
#
#
	.globl	Object.copy
Object.copy:			# self is in $a0
	addiu	$sp $sp -12	# frame size
	sw	$ra 4($sp)
	sw	$a0 8($sp)
	sw	$gp 12($sp)	# save ptr to new obj
	lw	$t0 obj_size($a0)	# Size of obj $t0
	sll	$t0 $t0 2	# Size is in words, *4
	add	$t1 $gp $t0	# where $gp will be after copy
	ble	$t1 $s7	_oc_ok	# $s7 is limit pointer

	# Need more memory
	la	$a0 _heap_msg
	li	$v0 4
	syscall
	li	$v0 9		# sbrk
	li	$a0 0x10000	# 64K bytes (larger obj. will fail)
	syscall
	li	$v0 9
	move	$a0 $zero
	syscall			# get new limit
	move	$s7 $v0		# update limit pointer
_oc_ok:
	lw	$a0 8($sp)	# restore obj ptr.
	lw	$t0 obj_size($a0)	# Size of obj $t0
_oc_loop:
	lw	$v0 0($a0)	# load a word
	addiu	$a0 4		# increment source
	addiu	$t0 -1		# decrement counter
	sw	$v0 0($gp)	# store word
	addiu	$gp 4		# increment dest
	bnez	$t0 _oc_loop

	lw	$a0 12($sp)	# the new object
	lw	$ra 4($sp)
	addiu	$sp $sp 12
	jr	$ra		# return


#
#
# Object.abort
#
#	The abort method for the object class (usually inherited by
#	all other classes)
#
#   INPUT:	$a0 contains the object on which abort() was dispatched.
#
	.globl	Object.abort

Object.abort:
	move	$s0 $a0		# save self
	li	$v0 4
	la	$a0 _abort_msg
	syscall			# print_str
	la	$t1 class_nameTab
	lw	$v0 obj_tag($s0)	# Get object tag
	sll	$v0 $v0 2	# *4
	addu	$t1 $t1 $v0
	lw	$t1 0($t1)	# Load class name string obj.
	addiu	$a0 $t1 str_field	# Adjust to beginning of str
	li	$v0 4		# print_str
	syscall
	la	$a0 _nl
	li	$v0 4
	syscall			# print new line
	li	$v0 10
	syscall			# Exit

#
#
# Object.type_name	
#
#   	INPUT:	$a0 object who's class name is desired
#	OUTPUT:	$a0 reference to class name string object
#
	.globl	Object.type_name
Object.type_name:
	la	$t1 class_nameTab
	lw	$v0 obj_tag($a0)	# Get object tag
	sll	$v0 $v0 2	# *4
	addu	$t1 $t1 $v0	# index table
	lw	$a0 0($t1)	# Load class name string obj.
	jr	$ra

#
#
# IO.out_string
#
#	Prints out the contents of a string object argument
#	which is on top of the stack.
#
#	$a0 is preserved!
#
	.globl	IO.out_string
IO.out_string:
	sw	$a0 0($sp)	# save self
	addiu	$sp $sp -4
	lw	$a0 8($sp)	# get arg
	addiu	$a0 $a0 str_field	# Adjust to beginning of str
	li	$v0 4		# print_str
	syscall	
	lw	$a0 4($sp)	# return self
	addiu	$sp $sp 8       # pop argument off stack
	jr	$ra

#
#
# IO.out_int
#
#	Prints out the contents of an integer object on top of the
#	stack.
#
#	$a0 is preserved!
#
	.globl	IO.out_int
IO.out_int:
	sw	$a0 0($sp)	# save self
	addiu	$sp $sp -4
	lw	$a0 8($sp)	# get arg
	lw	$a0 int_slot($a0)	# Fetch int
	li	$v0 1		# print_int
	syscall	
	lw	$a0 4($sp)	# return self
	addiu	$sp $sp 8
	jr	$ra

#
#
# IO.in_int
#
#	Returns an integer object read from the terminal in $a0
#
	.globl	IO.in_int
IO.in_int:
	sw	$ra 0($sp)	# save return address
	addiu	$sp $sp -4
        la      $a0 Int_protObj
        jal     Object.copy	# Call copy
        jal     Int_init
	sw	$a0 0($sp)	# save new object
	addiu	$sp $sp -4

	li	$v0, 5		# read int
	syscall

	lw	$a0 4($sp)
	addiu	$sp $sp 4
	sw	$v0 int_slot($a0)	# store int read into obj
	lw	$ra 4($sp)
	addiu	$sp $sp 4
	jr	$ra

#
#
# IO.in_string
#
#	Returns a string object read from the terminal, removing the
#	'\n'
#
#	OUTPUT:	$a0 the read string object
#
	.globl	IO.in_string
IO.in_string:
	sw	$ra 0($sp)	# save return address
	addiu	$sp $sp -4

        la      $a0 Int_protObj	# Create int object for size
        jal     Object.copy	# Call copy
        jal     Int_init
	sw	$a0 0($sp)	# save new object
	addiu	$sp $sp -4

        la      $a0 String_protObj
        jal     Object.copy	# Call copy
        jal     String_init

	addiu	$gp $gp -4	# Move back over last word of string

	sw	$a0 0($sp)	# save new object
	addiu	$sp $sp -4

	li	$a1 1026	# largest string to read
	add	$t1 $gp $a1
	ble	$t1 $s7	_is_ok	# $s7 is limit pointer

	# Need more memory
	la	$a0 _heap_msg
	li	$v0 4
	syscall
	li	$v0 9		# sbrk
	li	$a0 0x10000	# 64K bytes (larger obj. will fail)
	syscall
	li	$v0 9
	move	$a0 $zero
	syscall			# get new limit
	move	$s7 $v0		# update limit pointer
_is_ok:
	li	$a1 1026	# largest string to read
	move	$a0 $gp	
	li	$v0, 8		# read string
	syscall

	move	$t0 $gp		# t0 to beginning of string
_is_find_end:
	lb	$v0 0($gp)
	addiu	$gp $gp 1
	bnez	$v0 _is_find_end

	# $gp points just after the null byte
	lb	$v0 0($t0)	# is first byte '\0'?
	bnez	$v0 _is_noteof

	# we read nothing. Return '\n' (we don't have '\0'!!!)
	add	$v0 $zero 10	# load '\n' into $v0
	sb	$v0 -1($gp)
	sb	$zero 0($gp)	# terminate
	addiu	$gp $gp 1
	b	_is_nonl

_is_noteof:
	# Check if there really is a '\n'
	lb	$v0 -2($gp)
	bne	$v0 10 _is_nonl

	# Write '\0' over '\n'
	sb	$zero -2($gp)	# Set end of string where '\n' was
	addiu	$gp $gp -1	# adjust for '\n'

_is_nonl:
	lw	$a0 4($sp)	# get pointer to new str obj
	addiu	$sp $sp 4

	lw	$t1 4($sp)	# get pointer to new int obj
	addiu	$sp $sp 4
	sw	$t1 str_size($a0)	# Init size slot ptr

	sub	$t0 $gp $a0
	subu	$t0 str_field	# calc actual str size
	addiu	$t0  -1		# adjust for '\0'
	sw	$t0 int_slot($t1) # store string size in int obj
	addi	$gp $gp 3	# was already 1 past '\0'
	la	$t0 0xfffffffc
	and	$gp $gp $t0			# word align $gp
	# $gp is now word aligned

	sub	$t0 $gp $a0	# calc length
	srl	$t0 $t0  2	# divide by 4
	sw	$t0 obj_size($a0)	# set size field of obj

	lw	$ra 4($sp)
	addiu	$sp $sp 4
	jr	$ra


#
#
# String.length
#		Returns Int Obj with string length of self
#
#	INPUT:	$a0 the string object
#	OUTPUT:	$a0 the int object which is the size of the string
#
	.globl	String.length
String.length:
	lw	$a0 str_size($a0)	# fetch attr
	jr	$ra	# Return

#
#
# String.concat
#		Returns a the concatenation of self and arg1
#
#	INPUT:	$a0 the first string object
#		Top of stack: the second string object
#	OUTPUT:	$a0 the new string object
#
	.globl	String.concat
String.concat:
	addiu	$sp $sp -12	# frame
	sw	$a0 12($sp)	# save self arg.
	sw	$ra 8($sp)	# save return address
	
        lw      $a0 str_size($a0)
        jal     Object.copy	# Call copy
	sw	$a0 4($sp)	# save new size

	lw	$a0 12($sp)	# copy self
        jal     Object.copy	# Call copy
	sw	$a0 12($sp)	# save new string obj ptr

	lw	$t0 4($sp)	# new size pointer
	sw	$t0 str_size($a0) # install new size obj

	lw	$a1 16($sp)	# the argument string
	lw	$t1 str_size($a1)

	lw	$v0 int_slot($t0) # self string size
	lw	$v1 int_slot($t1) # arg string size
	addu	$t2 $v1 $v0	  # new size
	sw	$t2 int_slot($t0) # store new size

	addu	$t2 $a0 $v0	  # add 1st size to new ptr
	addiu	$t2 $t2 str_field # point to end of 1st string
	sw	$t2 4($sp)	  # save alloc pointer

	# Check for memory
	addu	$v0 $gp $v1
	ble	$v0 $s7	_sc_ok	# limit pointer

	# Need more memory
	la	$a0 _heap_msg
	li	$v0 4
	syscall
	li	$v0 9		# sbrk
	li	$a0 0x10000	# 64K bytes (larger obj. will fail)
	syscall
	li	$v0 9
	move	$a0 $zero
	syscall			# get new limit
	move	$s7 $v0		# update limit pointer
_sc_ok:
	lw	$a1 16($sp)	# get arg string obj
	lw	$a0 4($sp)	# the end of the 1st string
	lw	$v1 str_size($a1)
	lw	$v1 int_slot($v1) # size of second string
	beqz	$v1 _sc_end	# second string is empty
	add	$a1 $a1 str_field # a1 to beginning of 2nd string
_sc_loop:
	lb	$v0 0($a1)
	addiu	$a1 $a1 1	# inc src
	sb	$v0 0($a0)
	addiu	$a0 $a0 1	# inc dst
	addiu	$v1 $v1 -1	# dec ctr
	bnez	$v1 _sc_loop
_sc_end:
	sb	$zero 0($a0)	# null terminate
	move	$gp $a0
	addiu	$gp $gp 4	# realign the heap ptr
	la	$t0 0xfffffffc
	and	$gp $gp $t0			# word align $gp
	lw	$a0 12($sp)	# get new obj ptr
	sub	$t0 $gp $a0	# calc object size
	srl	$t0 $t0 2	# div by 4
	sw	$t0 obj_size($a0)

	lw	$ra 8($sp)
	addiu	$sp $sp 16
	jr	$ra

#
#
# String.substr(i,l)
#		Returns the sub string of self from i with length l
#		Offset starts at 0.
#
#	INPUT:	$a0 the string
#		index int object on top of stack (-4)
#		length int object below index on stack (-8)
#	OUTPUT:	The substring object in $a0
#
	.globl	String.substr
String.substr:
	addiu	$sp $sp -12	# frame
	sw	$ra 4($sp)	# save return
	sw	$a0 12($sp)	# save self
	lw	$v0 obj_size($a0)
	sll	$v0 $v0 2	# size in bytes
	add	$v0 $v0 $gp
	ble	$v0 $s7	_ss_ok	# Limit check

	# Need more memory
	la	$a0 _heap_msg
	li	$v0 4
	syscall
	li	$v0 9		# sbrk
	li	$a0 0x10000	# 64K bytes (larger obj. will fail)
	syscall
	li	$v0 9
	move	$a0 $zero
	syscall			# get new limit
	move	$s7 $v0		# update limit pointer
_ss_ok:
	la	$a0 Int_protObj
	jal	Object.copy
	jal	Int_init
	sw	$a0 8($sp)	# save new length obj
	la	$a0 String_protObj
	jal	Object.copy
	jal	String_init	# new obj ptr in $a0
	move	$a2 $a0		# use a2 to make copy
	addiu	$gp $gp -4	# backup alloc ptr
	lw	$a1 12($sp)	# load orig
	lw	$t1 16($sp)	# index obj
	lw	$t2 20($sp)	# length obj
	lw	$t0 str_size($a1)
	lw	$v1 int_slot($t1) # index
	lw	$v0 int_slot($t0) # size of orig
	bltz	$v1 _ss_abort1	# index is smaller than 0
	bgt	$v1 $v0 _ss_abort2	# index > orig
	lw	$t3 int_slot($t2) # sub length
	add	$v1 $v1 $t3	# index+sublength
	bgt	$v1 $v0 _ss_abort3
	bltz	$t3 _ss_abort4
	lw	$t4 8($sp)	# load new length obj
	sw	$t3 int_slot($t4) # save new size
	sw	$t4 str_size($a0) # store size in string
	lw	$v1 int_slot($t1) # index
	addiu	$a1 $a1 str_field # advance src to str
	add	$a1 $a1 $v1	  # advance to indexed char
	addiu	$a2 $a2 str_field # advance dst to str
	beqz	$t3 _ss_end	  # empty length
_ss_loop:
	lb	$v0 0($a1)
	addiu	$a1 $a1 1	# inc src
	sb	$v0 0($a2)
	addiu	$a2 $a2 1	# inc dst
	addiu	$t3 $t3 -1	# dec ctr
	bnez	$t3 _ss_loop
_ss_end:
	sb	$zero 0($a2)	# null terminate
	move	$gp $a2
	addiu	$gp $gp 4	# realign the heap ptr
	la	$t0 0xfffffffc
	and	$gp $gp $t0			# word align $gp

	sub	$t0 $gp $a0	# calc object size
	srl	$t0 $t0 2	# div by 4
	sw	$t0 obj_size($a0)

	lw	$ra 4($sp)
	addiu	$sp $sp 20
	jr	$ra

_ss_abort1:
	la	$a0 _sabort_msg1
	b	_ss_abort
_ss_abort2:
	la	$a0 _sabort_msg2
	b	_ss_abort
_ss_abort3:
	la	$a0 _sabort_msg3
	b	_ss_abort
_ss_abort4:
	la	$a0 _sabort_msg4
_ss_abort:
	li	$v0 4
	syscall
	la	$a0 _sabort_msg
	li	$v0 4
	syscall
	li	$v0 10		# exit
	syscall
