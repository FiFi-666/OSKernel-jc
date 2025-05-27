# Initial process that execs all.
# This code runs in user space.

#include "include/sysnum.h"

# exec(init, argv)
.globl start
start:
        li a0, 2    //O_RDWR
        la a1, 1    //console
        li a2, 0
        li a7, SYS_dev
        ecall	# dev(O_RDWR, CONSOLE, 0);
        
        li a0, 0
        li a7, SYS_dup
        ecall	# dup(0);  // stdout

        li a0, 0
        li a7, SYS_dup
        ecall	# dup(0);  // stderr
//运行time_test
	li a7, SYS_fork
	ecall
	bne a0, zero, after_time_test
	la a0, time_test
	la a1, argv_time_test
	li a7, SYS_exec
	ecall
after_time_test:
	li a0, 0
	li a7, SYS_wait
	ecall

//输出echo "run busybox_testcode.sh"
//echo_run_busybox_testcode
	li a7, SYS_fork
	ecall
	bne a0, zero, after_echo_run_busybox_testcode
	la a0, busybox
	la a1, argv_echo_run_busybox_testcode
	li a7, SYS_exec
	ecall
after_echo_run_busybox_testcode:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_0
	la a0, busybox
	la a1, argv_busybox_0
	li a7, SYS_exec
	ecall
after_0:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_1
	la a0, busybox
	la a1, argv_busybox_1
	li a7, SYS_exec
	ecall
after_1:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_2
	la a0, busybox
	la a1, argv_busybox_2
	li a7, SYS_exec
	ecall
after_2:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_3
	la a0, busybox
	la a1, argv_busybox_3
	li a7, SYS_exec
	ecall
after_3:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_4
	la a0, busybox
	la a1, argv_busybox_4
	li a7, SYS_exec
	ecall
after_4:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_5
	la a0, busybox
	la a1, argv_busybox_5
	li a7, SYS_exec
	ecall
after_5:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_6
	la a0, busybox
	la a1, argv_busybox_6
	li a7, SYS_exec
	ecall
after_6:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_7
	la a0, busybox
	la a1, argv_busybox_7
	li a7, SYS_exec
	ecall
after_7:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_8
	la a0, busybox
	la a1, argv_busybox_8
	li a7, SYS_exec
	ecall
after_8:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_9
	la a0, busybox
	la a1, argv_busybox_9
	li a7, SYS_exec
	ecall
after_9:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_10
	la a0, busybox
	la a1, argv_busybox_10
	li a7, SYS_exec
	ecall
after_10:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_11
	la a0, busybox
	la a1, argv_busybox_11
	li a7, SYS_exec
	ecall
after_11:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_12
	la a0, busybox
	la a1, argv_busybox_12
	li a7, SYS_exec
	ecall
after_12:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_13
	la a0, busybox
	la a1, argv_busybox_13
	li a7, SYS_exec
	ecall
after_13:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_14
	la a0, busybox
	la a1, argv_busybox_14
	li a7, SYS_exec
	ecall
after_14:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_15
	la a0, busybox
	la a1, argv_busybox_15
	li a7, SYS_exec
	ecall
after_15:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_16
	la a0, busybox
	la a1, argv_busybox_16
	li a7, SYS_exec
	ecall
after_16:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_17
	la a0, busybox
	la a1, argv_busybox_17
	li a7, SYS_exec
	ecall
after_17:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_18
	la a0, busybox
	la a1, argv_busybox_18
	li a7, SYS_exec
	ecall
after_18:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_19
	la a0, busybox
	la a1, argv_busybox_19
	li a7, SYS_exec
	ecall
after_19:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_20
	la a0, busybox
	la a1, argv_busybox_20
	li a7, SYS_exec
	ecall
after_20:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_21
	la a0, busybox
	la a1, argv_busybox_21
	li a7, SYS_exec
	ecall
after_21:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_22
	la a0, busybox
	la a1, argv_busybox_22
	li a7, SYS_exec
	ecall
after_22:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_23
	la a0, busybox
	la a1, argv_busybox_23
	li a7, SYS_exec
	ecall
after_23:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_24
	la a0, busybox
	la a1, argv_busybox_24
	li a7, SYS_exec
	ecall
after_24:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_25
	la a0, busybox
	la a1, argv_busybox_25
	li a7, SYS_exec
	ecall
after_25:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_26
	la a0, busybox
	la a1, argv_busybox_26
	li a7, SYS_exec
	ecall
after_26:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_27
	la a0, busybox
	la a1, argv_busybox_27
	li a7, SYS_exec
	ecall
after_27:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_28
	la a0, busybox
	la a1, argv_busybox_28
	li a7, SYS_exec
	ecall
after_28:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_29
	la a0, busybox
	la a1, argv_busybox_29
	li a7, SYS_exec
	ecall
after_29:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_30
	la a0, busybox
	la a1, argv_busybox_30
	li a7, SYS_exec
	ecall
after_30:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_31
	la a0, busybox
	la a1, argv_busybox_31
	li a7, SYS_exec
	ecall
after_31:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_32
	la a0, busybox
	la a1, argv_busybox_32
	li a7, SYS_exec
	ecall
after_32:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_33
	la a0, busybox
	la a1, argv_busybox_33
	li a7, SYS_exec
	ecall
after_33:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_34
	la a0, busybox
	la a1, argv_busybox_34
	li a7, SYS_exec
	ecall
after_34:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_35
	la a0, busybox
	la a1, argv_busybox_35
	li a7, SYS_exec
	ecall
after_35:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_36
	la a0, busybox
	la a1, argv_busybox_36
	li a7, SYS_exec
	ecall
after_36:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_37
	la a0, busybox
	la a1, argv_busybox_37
	li a7, SYS_exec
	ecall
after_37:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_38
	la a0, busybox
	la a1, argv_busybox_38
	li a7, SYS_exec
	ecall
after_38:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_39
	la a0, busybox
	la a1, argv_busybox_39
	li a7, SYS_exec
	ecall
after_39:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_40
	la a0, busybox
	la a1, argv_busybox_40
	li a7, SYS_exec
	ecall
after_40:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_41
	la a0, busybox
	la a1, argv_busybox_41
	li a7, SYS_exec
	ecall
after_41:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_42
	la a0, busybox
	la a1, argv_busybox_42
	li a7, SYS_exec
	ecall
after_42:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_43
	la a0, busybox
	la a1, argv_busybox_43
	li a7, SYS_exec
	ecall
after_43:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_44
	la a0, busybox
	la a1, argv_busybox_44
	li a7, SYS_exec
	ecall
after_44:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_45
	la a0, busybox
	la a1, argv_busybox_45
	li a7, SYS_exec
	ecall
after_45:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_46
	la a0, busybox
	la a1, argv_busybox_46
	li a7, SYS_exec
	ecall
after_46:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_47
	la a0, busybox
	la a1, argv_busybox_47
	li a7, SYS_exec
	ecall
after_47:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_48
	la a0, busybox
	la a1, argv_busybox_48
	li a7, SYS_exec
	ecall
after_48:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_49
	la a0, busybox
	la a1, argv_busybox_49
	li a7, SYS_exec
	ecall
after_49:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_50
	la a0, busybox
	la a1, argv_busybox_50
	li a7, SYS_exec
	ecall
after_50:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_51
	la a0, busybox
	la a1, argv_busybox_51
	li a7, SYS_exec
	ecall
after_51:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_52
	la a0, busybox
	la a1, argv_busybox_52
	li a7, SYS_exec
	ecall
after_52:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_53
	la a0, busybox
	la a1, argv_busybox_53
	li a7, SYS_exec
	ecall
after_53:
	li a0, 0
	li a7, SYS_wait
	ecall

	li a7, SYS_fork
	ecall
	bne a0, zero, after_54
	la a0, busybox
	la a1, argv_busybox_54
	li a7, SYS_exec
	ecall
after_54:
	li a0, 0
	li a7, SYS_wait
	ecall

loop:
        jal loop

# for(;;) exit();
program_exit:
        li a7, SYS_exit
        ecall
        jal program_exit

time_test:
	.string "/time-test\0"
argv_time_test:
	.dword time_test
	.dword 0

#echo "run busybox_testcode.sh"
param_busybox_testcode0:
	.string "echo\0"
param_busybox_testcode1:
	.string "busybox_testcode.sh\0"

argv_echo_run_busybox_testcode:
	.dword busybox
	.dword param_busybox_testcode0
	.dword param_busybox_testcode1
	.dword 0

busybox:
    .string "/busybox\0"
param00:
	.string "echo\0"
param01:
	.string "#### independent command test\0"
argv_busybox_0:
	.dword busybox
	.dword param00
	.dword param01
	.dword 0
param10:
	.string "ash\0"
param11:
	.string "-c\0"
param12:
	.string "exit\0"
argv_busybox_1:
	.dword busybox
	.dword param10
	.dword param11
	.dword param12
	.dword 0
param20:
	.string "sh\0"
param21:
	.string "-c\0"
param22:
	.string "exit\0"
argv_busybox_2:
	.dword busybox
	.dword param20
	.dword param21
	.dword param22
	.dword 0
param30:
	.string "basename\0"
param31:
	.string "/aaa/bbb\0"
argv_busybox_3:
	.dword busybox
	.dword param30
	.dword param31
	.dword 0
param40:
	.string "cal\0"
argv_busybox_4:
	.dword busybox
	.dword param40
	.dword 0
param50:
	.string "clear\0"
argv_busybox_5:
	.dword busybox
	.dword param50
	.dword 0
param60:
	.string "date\0"
argv_busybox_6:
	.dword busybox
	.dword param60
	.dword 0
param70:
	.string "df\0"
argv_busybox_7:
	.dword busybox
	.dword param70
	.dword 0
param80:
	.string "dirname\0"
param81:
	.string "/aaa/bbb\0"
argv_busybox_8:
	.dword busybox
	.dword param80
	.dword param81
	.dword 0
param90:
	.string "dmesg\0"
argv_busybox_9:
	.dword busybox
	.dword param90
	.dword 0
param100:
	.string "du\0"
argv_busybox_10:
	.dword busybox
	.dword param100
	.dword 0
param110:
	.string "expr\0"
param111:
	.string "1\0"
param112:
	.string "+\0"
param113:
	.string "1\0"
argv_busybox_11:
	.dword busybox
	.dword param110
	.dword param111
	.dword param112
	.dword param113
	.dword 0
param120:
	.string "false\0"
argv_busybox_12:
	.dword busybox
	.dword param120
	.dword 0
param130:
	.string "true\0"
argv_busybox_13:
	.dword busybox
	.dword param130
	.dword 0
param140:
	.string "which\0"
param141:
	.string "ls\0"
argv_busybox_14:
	.dword busybox
	.dword param140
	.dword param141
	.dword 0
param150:
	.string "uname\0"
argv_busybox_15:
	.dword busybox
	.dword param150
	.dword 0
param160:
	.string "uptime\0"
argv_busybox_16:
	.dword busybox
	.dword param160
	.dword 0
param170:
	.string "printf\0"
param171:
	.string "abc\n\0"
argv_busybox_17:
	.dword busybox
	.dword param170
	.dword param171
	.dword 0
param180:
	.string "ps\0"
argv_busybox_18:
	.dword busybox
	.dword param180
	.dword 0
param190:
	.string "pwd\0"
argv_busybox_19:
	.dword busybox
	.dword param190
	.dword 0
param200:
	.string "free\0"
argv_busybox_20:
	.dword busybox
	.dword param200
	.dword 0
param210:
	.string "hwclock\0"
argv_busybox_21:
	.dword busybox
	.dword param210
	.dword 0
param220:
	.string "kill\0"
param221:
	.string "10\0"
argv_busybox_22:
	.dword busybox
	.dword param220
	.dword param221
	.dword 0
param230:
	.string "ls\0"
argv_busybox_23:
	.dword busybox
	.dword param230
	.dword 0
param240:
	.string "sleep\0"
param241:
	.string "1\0"
argv_busybox_24:
	.dword busybox
	.dword param240
	.dword param241
	.dword 0
param250:
	.string "echo\0"
param251:
	.string "#### file opration test\0"
argv_busybox_25:
	.dword busybox
	.dword param250
	.dword param251
	.dword 0
param260:
	.string "touch\0"
param261:
	.string "test.txt\0"
argv_busybox_26:
	.dword busybox
	.dword param260
	.dword param261
	.dword 0
param270:
	.string "echo\0"
param271:
	.string "hello world\0"
param272:
	.string ">\0"
param273:
	.string "test.txt\0"
argv_busybox_27:
	.dword busybox
	.dword param270
	.dword param271
	.dword param272
	.dword param273
	.dword 0
param280:
	.string "cat\0"
param281:
	.string "test.txt\0"
argv_busybox_28:
	.dword busybox
	.dword param280
	.dword param281
	.dword 0
param290:
	.string "cut\0"
param291:
	.string "-c\0"
param292:
	.string "3\0"
param293:
	.string "test.txt\0"
argv_busybox_29:
	.dword busybox
	.dword param290
	.dword param291
	.dword param292
	.dword param293
	.dword 0
param300:
	.string "od\0"
param301:
	.string "test.txt\0"
argv_busybox_30:
	.dword busybox
	.dword param300
	.dword param301
	.dword 0
param310:
	.string "head\0"
param311:
	.string "test.txt\0"
argv_busybox_31:
	.dword busybox
	.dword param310
	.dword param311
	.dword 0
param320:
	.string "tail\0"
param321:
	.string "test.txt\0"
argv_busybox_32:
	.dword busybox
	.dword param320
	.dword param321
	.dword 0
param330:
	.string "hexdump\0"
param331:
	.string "-C\0"
param332:
	.string "test.txt\0"
argv_busybox_33:
	.dword busybox
	.dword param330
	.dword param331
	.dword param332
	.dword 0
param340:
	.string "md5sum\0"
param341:
	.string "test.txt\0"
argv_busybox_34:
	.dword busybox
	.dword param340
	.dword param341
	.dword 0
param350:
	.string "echo\0"
param351:
	.string "ccccccc\0"
param352:
	.string ">>\0"
param353:
	.string "test.txt\0"
argv_busybox_35:
	.dword busybox
	.dword param350
	.dword param351
	.dword param352
	.dword param353
	.dword 0
param360:
	.string "echo\0"
param361:
	.string "bbbbbbb\0"
param362:
	.string ">>\0"
param363:
	.string "test.txt\0"
argv_busybox_36:
	.dword busybox
	.dword param360
	.dword param361
	.dword param362
	.dword param363
	.dword 0
param370:
	.string "echo\0"
param371:
	.string "aaaaaaa\0"
param372:
	.string ">>\0"
param373:
	.string "test.txt\0"
argv_busybox_37:
	.dword busybox
	.dword param370
	.dword param371
	.dword param372
	.dword param373
	.dword 0
param380:
	.string "echo\0"
param381:
	.string "2222222\0"
param382:
	.string ">>\0"
param383:
	.string "test.txt\0"
argv_busybox_38:
	.dword busybox
	.dword param380
	.dword param381
	.dword param382
	.dword param383
	.dword 0
param390:
	.string "echo\0"
param391:
	.string "1111111\0"
param392:
	.string ">>\0"
param393:
	.string "test.txt\0"
argv_busybox_39:
	.dword busybox
	.dword param390
	.dword param391
	.dword param392
	.dword param393
	.dword 0
param400:
	.string "echo\0"
param401:
	.string "bbbbbbb\0"
param402:
	.string ">>\0"
param403:
	.string "test.txt\0"
argv_busybox_40:
	.dword busybox
	.dword param400
	.dword param401
	.dword param402
	.dword param403
	.dword 0
param410:
	.string "sort\0"
param411:
	.string "test.txt\0"
param412:
	.string "|\0"
param413:
	.string "./busybox\0"
param414:
	.string "uniq\0"
argv_busybox_41:
	.dword busybox
	.dword param410
	.dword param411
	.dword param412
	.dword param413
	.dword param414
	.dword 0
param420:
	.string "stat\0"
param421:
	.string "test.txt\0"
argv_busybox_42:
	.dword busybox
	.dword param420
	.dword param421
	.dword 0
param430:
	.string "strings\0"
param431:
	.string "test.txt\0"
argv_busybox_43:
	.dword busybox
	.dword param430
	.dword param431
	.dword 0
param440:
	.string "wc\0"
param441:
	.string "test.txt\0"
argv_busybox_44:
	.dword busybox
	.dword param440
	.dword param441
	.dword 0
param450:
	.string "[\0"
param451:
	.string "-f\0"
param452:
	.string "test.txt\0"
param453:
	.string "]\0"
argv_busybox_45:
	.dword busybox
	.dword param450
	.dword param451
	.dword param452
	.dword param453
	.dword 0
param460:
	.string "more\0"
param461:
	.string "test.txt\0"
argv_busybox_46:
	.dword busybox
	.dword param460
	.dword param461
	.dword 0
param470:
	.string "rm\0"
param471:
	.string "test.txt\0"
argv_busybox_47:
	.dword busybox
	.dword param470
	.dword param471
	.dword 0
param480:
	.string "mkdir\0"
param481:
	.string "test_dir\0"
argv_busybox_48:
	.dword busybox
	.dword param480
	.dword param481
	.dword 0
param490:
	.string "mv\0"
param491:
	.string "test_dir\0"
param492:
	.string "test\0"
argv_busybox_49:
	.dword busybox
	.dword param490
	.dword param491
	.dword param492
	.dword 0
param500:
	.string "rmdir\0"
param501:
	.string "test\0"
argv_busybox_50:
	.dword busybox
	.dword param500
	.dword param501
	.dword 0
param510:
	.string "grep\0"
param511:
	.string "hello\0"
param512:
	.string "busybox_cmd.txt\0"
argv_busybox_51:
	.dword busybox
	.dword param510
	.dword param511
	.dword param512
	.dword 0
param520:
	.string "cp\0"
param521:
	.string "busybox_cmd.txt\0"
param522:
	.string "busybox_cmd.bak\0"
argv_busybox_52:
	.dword busybox
	.dword param520
	.dword param521
	.dword param522
	.dword 0
param530:
	.string "rm\0"
param531:
	.string "busybox_cmd.bak\0"
argv_busybox_53:
	.dword busybox
	.dword param530
	.dword param531
	.dword 0
param540:
	.string "find\0"
param541:
	.string "-name\0"
param542:
	.string "busybox_cmd.txt\0"
argv_busybox_54:
	.dword busybox
	.dword param540
	.dword param541
	.dword param542
	.dword 0



