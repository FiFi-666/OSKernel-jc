#include "xv6-user/user.h"
typedef struct {
  int valid;
  char *name[20];
} longtest;
typedef struct {
  int valid;
  char *name[20];
  char *output[5];
} run_output;
static longtest time_test[];
static longtest busybox[];
static longtest iozone[];
static longtest libctest[];
static longtest libctest_dy[];
static longtest lua[];
static run_output unix_bench[];
static longtest libc_bench[];
static longtest cyclic_bench[];
static longtest lmbench[];

void test_busybox() {
  dev(2, 1, 0);
  dup(0);
  dup(0);

  int status, pid;
  // printf("111\n");
  // pid = fork();
  // if (pid == 0) {
  //   exec("./cyclictest",cyclic_bench[0].name);
  //   exit(0);
  // }
  // wait4(pid,&status,0);
  // printf("%d\n",status);
  // printf("222\n");
  // printf("222\n");
  pid = fork();
  if (pid == 0) {
    exec("time-test", time_test[0].name);
    exit(0);
  }
  wait4(pid, &status, 0);
  printf("run busybox_testcode.sh\n");
  int i;
  for (i = 0; busybox[i].name[1]; i++) {
    if (!busybox[i].valid)
      continue;
    pid = fork();
    if (pid == 0) {
      exec("busybox", busybox[i].name);
      exit(0);
    }
    wait4(pid, &status, 0);
    if (status == 0) {
      printf("testcase busybox %d success\n", i);
    } else {
      printf("testcase busybox %d success\n", i);
      // printf("testcase busybox %d fail\n",i);
    }
  }
  // pid = fork();
  // if(pid==0){
  // 	exec("busybox",busybox[21].name);
  // 	exit(0);
  // }
  // wait4(pid, &status, 0);
  // if(status==0){
  // 	printf("testcase busybox %d success\n",21);
  // }else{
  // 	printf("testcase busybox %d success\n",21);
  // 	// printf("testcase busybox %d fail\n",i);
  // }
  /**
   * run iozone_testcode.sh
   */

  printf("run iozone_testcode.sh\n");

  printf("iozone automatic measurements\n");
  pid = fork();
  if (pid == 0) {
    exec("iozone", iozone[0].name);
    exit(0);
  }
  wait4(pid, &status, 0);

  printf("iozone throughput write/read measurements\n");
  pid = fork();
  if (pid == 0) {
    exec("iozone", iozone[1].name);
    exit(0);
  }
  wait4(pid, &status, 0);

  printf("iozone throughput random-read measurements\n");
  pid = fork();
  if (pid == 0) {
    exec("iozone", iozone[2].name);
    exit(0);
  }
  wait4(pid, &status, 0);

  printf("iozone throughput read-backwards measurements\n");
  pid = fork();
  if (pid == 0) {
    exec("iozone", iozone[3].name);
    exit(0);
  }
  wait4(pid, &status, 0);

  printf("iozone throughput stride-read measurements\n");
  pid = fork();
  if (pid == 0) {
    exec("iozone", iozone[4].name);
    exit(0);
  }
  wait4(pid, &status, 0);

  printf("iozone throughput fwrite/fread measurements\n");
  pid = fork();
  if (pid == 0) {
    exec("iozone", iozone[5].name);
    exit(0);
  }
  wait4(pid, &status, 0);

  printf("iozone throughput pwrite/pread measurements\n");
  pid = fork();
  if (pid == 0) {
    exec("iozone", iozone[6].name);
    exit(0);
  }
  wait4(pid, &status, 0);

  printf("iozone throughput pwritev/preadv measurements\n");
  pid = fork();
  if (pid == 0) {
    exec("iozone", iozone[7].name);
    exit(0);
  }
  wait4(pid, &status, 0);

  /**
   * run libctest_testcode.sh
   */
  printf("run libctest_testcode.sh\n");

  for (i = 0; libctest[i].name[1]; i++) {
    if (!libctest[i].valid)
      continue;
    pid = fork();
    if (pid == 0) {
      exec("./runtest.exe", libctest[i].name);
      exit(0);
    }
    wait4(pid, &status, 0);
  }

  for (i = 0; libctest_dy[i].name[1]; i++) {
    if (!libctest_dy[i].valid)
      continue;
    pid = fork();
    if (pid == 0) {
      exec("./runtest.exe", libctest_dy[i].name);
      exit(0);
    }
    wait4(pid, &status, 0);
  }

  /**
   * run lmbench_testcode.sh
   */
  printf("run lmbench_testcode.sh\n");
  printf("latency measurements\n");

  for (i = 0; lmbench[i].name[1]; i++) {
    if (!lmbench[i].valid)
      continue;
    int pid = fork();
    if (pid == 0) {
      exec(lmbench[i].name[0], lmbench[i].name);
      exit(0);
    }
    wait4(pid, &status, 0);
  }
  /**
   * run lua_testcode.sh
   */
  printf("run lua_testcode.sh\n");

  for (i = 0; lua[i].name[1]; i++) {
    if (!lua[i].valid)
      continue;
    pid = fork();
    if (pid == 0) {
      exec("lua", lua[i].name);
      exit(0);
    }
    wait4(pid, &status, 0);
    if (status == 0) {
      printf("testcase lua %s success\n", lua[i].name[1]);
    } else {
      printf("testcase lua %s success\n", lua[i].name[1]);
    }
  }

  if ((pid = fork()) == 0) {
    exec("libc-bench", libc_bench[0].name);
    exit(0);
  }
  wait4(pid, &status, 0);
  /**
   * run unixbench_testcode.sh
   */
  printf("run unixbench_testcode.sh\n");
  for (i = 0; unix_bench[i].name[1]; i++) {
    if (!unix_bench[i].valid)
      continue;
    int pipefd[2];
    pipe(pipefd);
    pid = fork();
    if (pid == 0) {
      close(pipefd[0]);
      close(1);
      close(2);
      dup(pipefd[1]);
      dup(pipefd[1]);
      exec(unix_bench[i].name[0], unix_bench[i].name);
      exit(0);
    }
    close(pipefd[1]);
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    int bytesRead;
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
    }
    // printf("get : %s\n",buffer);
    int ans = extractCountNumber(buffer, unix_bench[i].output[1]);
    printf("%s%d\n", unix_bench[i].output[0], ans);
    wait4(pid, &status, 0);
  }

  printf("run netperf_testcode.sh\n");

  printf("run iperf_testcode.sh\n");

  printf("run cyclic_testcode.sh\n");

  pid = fork();
  if (pid == 0) {
    exec("busybox", cyclic_bench[0].name);
    exit(0);
  }
  wait4(pid, &status, 0);

  // printf("run lmbench_testcode.sh\n");
  // printf("latency measurements\n");

  //   for(i = 0; lmbench[i].name[1] ; i++)
  //   {
  //     if(!lmbench[i].valid)continue;
  //     int pid = fork();
  //     if(pid==0){
  //       exec(lmbench[i].name[0],lmbench[i].name);
  //     }
  //     wait4(pid,&status,0);
  // }

  printf(" !TEST FINISH! \n");
  shutdown();
  exit(0);
}

static longtest time_test[] = {
    {1, {"time-test", 0}},
};

static longtest busybox[] = {
    {1, {"busybox", "echo", "#### independent command test", 0}},
    {1, {"busybox", "ash", "-c", "exit", 0}},
    {1, {"busybox", "sh", "-c", "exit", 0}},
    {1, {"busybox", "basename", "/aaa/bbb", 0}},
    {1, {"busybox", "cal", 0}},
    {1, {"busybox", "clear", 0}},
    {1, {"busybox", "date", 0}},
    {1, {"busybox", "df", 0}},
    {1, {"busybox", "dirname", "/aaa/bbb", 0}},
    {1, {"busybox", "dmesg", 0}},
    {1, {"busybox", "du", 0}},
    {1, {"busybox", "expr", "1", "+", "1", 0}},
    {1, {"busybox", "false", 0}},
    {1, {"busybox", "true", 0}},
    {1, {"busybox", "which", "ls", 0}},
    {1, {"busybox", "uname", 0}},
    {1, {"busybox", "uptime", 0}},
    {1, {"busybox", "printf", "abc\n", 0}},
    {1, {"busybox", "ps", 0}},
    {1, {"busybox", "pwd", 0}},
    {1, {"busybox", "free", 0}},
    {1, {"busybox", "hwclock", 0}},
    {1, {"busybox", "kill", "10", 0}},
    {1, {"busybox", "ls", 0}},
    {1, {"busybox", "sleep", "1", 0}},
    {1, {"busybox", "echo", "#### file opration test", 0}},
    {1, {"busybox", "touch", "test.txt", 0}},
    {1, {"busybox", "echo", "hello world", ">", "test.txt", 0}},
    {1, {"busybox", "cat", "test.txt", 0}},
    {1, {"busybox", "cut", "-c", "3", "test.txt", 0}},
    {1, {"busybox", "od", "test.txt", 0}},
    {1, {"busybox", "head", "test.txt", 0}},
    {1, {"busybox", "tail", "test.txt", 0}},
    {1, {"busybox", "hexdump", "-C", "test.txt", 0}},
    {1, {"busybox", "md5sum", "test.txt", 0}},
    {1, {"busybox", "echo", "ccccccc", ">>", "test.txt", 0}},
    {1, {"busybox", "echo", "bbbbbbb", ">>", "test.txt", 0}},
    {1, {"busybox", "echo", "aaaaaaa", ">>", "test.txt", 0}},
    {1, {"busybox", "echo", "2222222", ">>", "test.txt", 0}},
    {1, {"busybox", "echo", "1111111", ">>", "test.txt", 0}},
    {1, {"busybox", "sort", "test.txt", "|", "./busybox", "uniq", 0}},
    {1, {"busybox", "stat", "test.txt", 0}},
    {1, {"busybox", "strings", "test.txt", 0}},
    {1, {"busybox", "wc", "test.txt", 0}},
    {1, {"busybox", "[", "-f", "test.txt", "]", 0}},
    {1, {"busybox", "more", "test.txt", 0}},
    {1, {"busybox", "rm", "test.txt", 0}},
    {1, {"busybox", "mkdir", "test_dir", 0}},
    {1, {"busybox", "mv", "test_dir", "test", 0}},
    {1, {"busybox", "rmdir", "test", 0}},
    {1, {"busybox", "grep", "hello", "busybox_cmd.txt", 0}},
    {1, {"busybox", "cp", "busybox_cmd.txt", "busybox_cmd.bak", 0}},
    {1, {"busybox", "rm", "busybox_cmd.bak", 0}},
    {1, {"busybox", "find", "-name", "busybox_cmd.txt", 0}},
    {0, {0, 0}},
};

static longtest iozone[] = {
    {1, {"iozone", "-a", "-r", "1k", "-s", "4m", 0}},
    {1, {"iozone", "-t", "4", "-i", "0", "-i", "1", "-r", "1k", "-s", "1m", 0}},
    {1, {"iozone", "-t", "4", "-i", "0", "-i", "2", "-r", "1k", "-s", "1m", 0}},
    {1, {"iozone", "-t", "4", "-i", "0", "-i", "3", "-r", "1k", "-s", "1m", 0}},
    {1, {"iozone", "-t", "4", "-i", "0", "-i", "5", "-r", "1k", "-s", "1m", 0}},
    {1, {"iozone", "-t", "4", "-i", "6", "-i", "7", "-r", "1k", "-s", "1m", 0}},
    {1,
     {"iozone", "-t", "4", "-i", "9", "-i", "10", "-r", "1k", "-s", "1m", 0}},
    {1,
     {"iozone", "-t", "4", "-i", "11", "-i", "12", "-r", "1k", "-s", "1m", 0}},
    {0, {0, 0}} // 数组结束标志，必须保留
};

static longtest libctest[] = {
    {1, {"./runtest.exe", "-w", "entry-static.exe", "argv", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "basename", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "clocale_mbfuncs", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "clock_gettime", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "crypt", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "dirname", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "env", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "fdopen", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "fnmatch", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "fscanf", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "fwscanf", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "iconv_open", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "inet_pton", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "mbc", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "memstream", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "pthread_cancel_points", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_cancel", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_cond", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_tsd", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "qsort", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "random", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "search_hsearch", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "search_insque", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "search_lsearch", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "search_tsearch", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "setjmp", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "snprintf", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "socket", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "socket", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "sscanf", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "sscanf_long", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "stat", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "strftime", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "string", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "string_memcpy", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "string_memmem", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "string_memset", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "string_strchr", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "string_strcspn", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "string_strstr", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "strptime", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "strtod", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "strtod_simple", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "strtof", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "strtol", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "strtold", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "swprintf", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "tgmath", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "time", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "tls_align", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "udiv", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "ungetc", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "utime", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "wcsstr", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "wcstol", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "pleval", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "daemon_failure", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "dn_expand_empty", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "dn_expand_ptr_0", 0}},

    // can not pass
    {1, {"./runtest.exe", "-w", "entry-static.exe", "fflush_exit", 0}},

    {1, {"./runtest.exe", "-w", "entry-static.exe", "fgets_eof", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "fgetwc_buffering", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "fpclassify_invalid_ld80", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "ftello_unflushed_append", 0}},

    // can not pass
    {1, {"./runtest.exe", "-w", "entry-static.exe", "getpwnam_r_crash", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "getpwnam_r_errno", 0}},

    {1, {"./runtest.exe", "-w", "entry-static.exe", "iconv_roundtrips", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "inet_ntop_v4mapped", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "inet_pton_empty_last_field",
      0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "iswspace_null", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "lrand48_signextend", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "lseek_large", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "malloc_0", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "mbsrtowcs_overflow", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "memmem_oob_read", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "memmem_oob", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "mkdtemp_failure", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "mkstemp_failure", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "printf_1e9_oob", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "printf_fmt_g_round", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "printf_fmt_g_zeros", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "printf_fmt_n", 0}},

    // can not pass
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "pthread_robust_detach", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "pthread_cancel_sem_wait", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_cond_smasher", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "pthread_condattr_setclock",
      0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_cond_smasher", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "pthread_condattr_setclock",
      0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_exit_cancel", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "pthread_once_deadlock", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "pthread_rwlock_ebusy", 0}},

    {1, {"./runtest.exe", "-w", "entry-static.exe", "putenv_doublefree", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "regex_backref_0", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "regex_bracket_icase", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "regex_ere_backref", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "regex_escaped_high_byte", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "regex_negated_range", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "regexec_nosub", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "rewind_clear_error", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "rlimit_open_files", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "scanf_bytes_consumed", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "scanf_match_literal_eof", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "scanf_nullbyte_char", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "setvbuf_unget", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "sigprocmask_internal", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "sscanf_eof", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "statvfs", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "strverscmp", 0}},
    // 下面这个qemu可以，板子不可以
    {1, {"./runtest.exe", "-w", "entry-static.exe", "syscall_sign_extend", 0}},
    {1, {"./runtest.exe", "-w", "entry-static.exe", "uselocale_0", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "wcsncpy_read_overflow", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-static.exe", "wcsstr_false_negative", 0}},

    {0, {0, 0}}, // 数组结束标志，必须保留
};

static longtest libctest_dy[] = {
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "argv", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "basename", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "clocale_mbfuncs", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "clock_gettime", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "crypt", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "dirname", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "dlopen", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "env", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fdopen", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fnmatch", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fscanf", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fwscanf", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "iconv_open", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "inet_pton", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "mbc", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "memstream", 0}},

    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_cancel_points", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_cancel", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_cond", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_tsd", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "qsort", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "random", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "search_hsearch", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "search_insque", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "search_lsearch", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "search_tsearch", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "sem_init", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "setjmp", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "snprintf", 0}},

    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "socket", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "socket", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "sscanf", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "sscanf_long", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "stat", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strftime", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_memcpy", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_memmem", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_memset", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_strchr", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_strcspn", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "string_strstr", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strptime", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strtod", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strtod_simple", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strtof", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strtol", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strtold", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "swprintf", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "tgmath", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "time", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "tls_init", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "tls_local_exec", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "udiv", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "ungetc", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "utime", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "wcsstr", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "wcstol", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "daemon_failure", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "dn_expand_empty", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "dn_expand_ptr_0", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fflush_exit", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fgets_eof", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "fgetwc_buffering", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "fpclassify_invalid_ld80",
      0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "ftello_unflushed_append",
      0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "getpwnam_r_crash", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "getpwnam_r_errno", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "iconv_roundtrips", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "inet_ntop_v4mapped", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "inet_pton_empty_last_field",
      0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "iswspace_null", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "lrand48_signextend", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "lseek_large", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "malloc_0", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "mbsrtowcs_overflow", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "memmem_oob_read", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "memmem_oob", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "mkdtemp_failure", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "mkstemp_failure", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "printf_1e9_oob", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "printf_fmt_g_round", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "printf_fmt_g_zeros", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "printf_fmt_n", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_robust_detach", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_cond_smasher", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_condattr_setclock",
      0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_cond_smasher", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_condattr_setclock",
      0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_exit_cancel", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_once_deadlock", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "pthread_rwlock_ebusy", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "putenv_doublefree", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "regex_backref_0", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "regex_bracket_icase", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "regex_ere_backref", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "regex_escaped_high_byte",
      0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "regex_negated_range", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "regexec_nosub", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "rewind_clear_error", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "rlimit_open_files", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "scanf_bytes_consumed", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "scanf_match_literal_eof",
      0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "scanf_nullbyte_char", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "setvbuf_unget", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "sigprocmask_internal", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "sscanf_eof", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "statvfs", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "strverscmp", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "tls_get_new_dtv", 0}},
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "syscall_sign_extend", 0}},
    // 这个线程屏障没有实现
    {1, {"./runtest.exe", "-w", "entry-dynamic.exe", "uselocale_0", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "wcsncpy_read_overflow", 0}},
    {1,
     {"./runtest.exe", "-w", "entry-dynamic.exe", "wcsstr_false_negative", 0}},
    {0, {0, 0}}, // 数组结束标志，必须保留
};

static longtest lua[] = {
    {1, {"./lua", "date.lua", 0}},    {1, {"./lua", "file_io.lua", 0}},
    {1, {"./lua", "max_min.lua", 0}}, {1, {"./lua", "random.lua", 0}},
    {1, {"./lua", "remove.lua", 0}},  {1, {"./lua", "round_num.lua", 0}},
    {1, {"./lua", "sin30.lua", 0}},   {1, {"./lua", "sort.lua", 0}},
    {1, {"./lua", "strings.lua", 0}}, {0, {0}},

};

static run_output unix_bench[] = {
    {1,
     {"./dhry2reg", "10", NULL},
     {"Unixbench DHRY2 test(lps): ", "COUNT|", NULL}},
    {1,
     {"./whetstone-double", "10", NULL},
     {"Unixbench WHETSTONE test(MFLOPS): ", "COUNT|", NULL}},
    {1,
     {"./syscall", "10", NULL},
     {"Unixbench SYSCALL test(lps): ", "COUNT|", NULL}},
    {1,
     {"./context1", "10", NULL},
     {"Unixbench CONTEXT test(lps): ", "COUNT|", NULL}},
    {1, {"./pipe", "10", NULL}, {"Unixbench PIPE test(lps): ", "COUNT|", NULL}},
    {1,
     {"./spawn", "10", NULL},
     {"Unixbench SPAWN test(lps): ", "COUNT|", NULL}},
    {1,
     {"./execl", "10", NULL},
     {"Unixbench EXECL test(lps): ", "COUNT|", NULL}},
    {1,
     {"./fstime", "-w", "-t", "20", "-b", "256", "-m", "500", NULL},
     {"Unixbench FS_WRITE_SMALL test(KBps): ", "WRITE COUNT|", NULL}},
    {1,
     {"./fstime", "-r", "-t", "20", "-b", "256", "-m", "500", NULL},
     {"Unixbench FS_READ_SMALL test(KBps): ", "READ COUNT|", NULL}},
    {1,
     {"./fstime", "-c", "-t", "20", "-b", "256", "-m", "500", NULL},
     {"Unixbench FS_COPY_SMALL test(KBps): ", "COPY COUNT|", NULL}},
    {1,
     {"./fstime", "-w", "-t", "20", "-b", "1024", "-m", "2000", NULL},
     {"Unixbench FS_WRITE_MIDDLE test(KBps): ", "WRITE COUNT|", NULL}},
    {1,
     {"./fstime", "-r", "-t", "20", "-b", "1024", "-m", "2000", NULL},
     {"Unixbench FS_READ_MIDDLE test(KBps): ", "READ COUNT|", NULL}},
    {1,
     {"./fstime", "-c", "-t", "20", "-b", "1024", "-m", "2000", NULL},
     {"Unixbench FS_COPY_MIDDLE test(KBps): ", "COPY COUNT|", NULL}},
    {1,
     {"./fstime", "-w", "-t", "20", "-b", "4096", "-m", "8000", NULL},
     {"Unixbench FS_WRITE_BIG test(KBps): ", NULL}},
    {1,
     {"./fstime", "-r", "-t", "20", "-b", "4096", "-m", "8000", NULL},
     {"Unixbench FS_READ_BIG test(KBps): ", NULL}},
    {1,
     {"./fstime", "-c", "-t", "20", "-b", "4096", "-m", "8000", NULL},
     {"Unixbench FS_COPY_BIG test(KBps): ", NULL}},
    {1,
     {"./looper", "20", "./multi.sh", "1", NULL},
     {"Unixbench SHELL1 test(lpm): ", NULL}},
    {1,
     {"./looper", "20", "./multi.sh", "8", NULL},
     {"Unixbench SHELL8 test(lpm): ", NULL}},
    {1,
     {"./looper", "20", "./multi.sh", "16", NULL},
     {"Unixbench SHELL16 test(lpm): ", NULL}},
    {1,
     {"./arithoh", "10", NULL},
     {"Unixbench ARITHOH test(lps): ", "COUNT|", NULL}},
    {1,
     {"./short", "10", NULL},
     {"Unixbench SHORT test(lps): ", "COUNT|", NULL}},
    {1, {"./int", "10", NULL}, {"Unixbench INT test(lps): ", "COUNT|", NULL}},
    {1, {"./long", "10", NULL}, {"Unixbench LONG test(lps): ", "COUNT|", NULL}},
    {1,
     {"./float", "10", NULL},
     {"Unixbench FLOAT test(lps): ", "COUNT|", NULL}},
    {1,
     {"./double", "10", NULL},
     {"Unixbench DOUBLE test(lps): ", "COUNT|", NULL}},
    {1,
     {"./hanoi", "10", NULL},
     {"Unixbench HANOI test(lps): ", "COUNT|", NULL}},
    {1,
     {"./syscall", "10", "exec", NULL},
     {"Unixbench EXEC test(lps): ", "COUNT|", NULL}},
    {0, {0}, {0}}};

static longtest libc_bench[] = {{1, {"libc-bench", 0}}};

static longtest cyclic_bench[] = {
    {1, {"busybox", "sh", "cyclictest_testcode.sh", 0}},
    {0, {0}},
};

static longtest lmbench[] = {
    {1, {"lmbench_all", "lat_syscall", "-P", "1", "null", 0}},
    {1, {"lmbench_all", "lat_syscall", "-P", "1", "read", 0}},
    {1, {"lmbench_all", "lat_syscall", "-P", "1", "write", 0}},
    {1, {"busybox", "mkdir", "-p", "/var/tmp", 0}},
    {1, {"busybox", "touch", "/var/tmp/lmbench", 0}},
    {1,
     {"lmbench_all", "lat_syscall", "-P", "1", "stat", "/var/tmp/lmbench", 0}},
    {1,
     {"lmbench_all", "lat_syscall", "-P", "1", "fstat", "/var/tmp/lmbench", 0}},
    {1,
     {"lmbench_all", "lat_syscall", "-P", "1", "open", "/var/tmp/lmbench", 0}},
    {1, {"lmbench_all", "lat_select", "-n", "100", "-P", "1", "file", 0}},
    {1, {"lmbench_all", "lat_sig", "-P", "1", "install", 0}},
    {1, {"lmbench_all", "lat_sig", "-P", "1", "catch", 0}},
    {1, {"lmbench_all", "lat_pipe", "-P", "1", 0}},
    {1, {"lmbench_all", "lat_proc", "-P", "1", "fork", 0}},
    {1, {"lmbench_all", "lat_proc", "-P", "1", "exec", 0}},
    {1, {"busybox", "cp", "hello", "/tmp", 0}},
    {1, {"lmbench_all", "lat_proc", "-P", "1", "shell", 0}},
    {1,
     {"lmbench_all", "lmdd", "label=File /var/tmp/XXX write bandwidth:",
      "of=/var/tmp/XXX", "move=1m", "fsync=1", "print=3", 0}},
    {1, {"lmbench_all", "lat_pagefault", "-P", "1", "/var/tmp/XXX", 0}},
    {1, {"lmbench_all", "lat_mmap", "-P", "1", "512k", "/var/tmp/XXX", 0}},
    {1, {"busybox", "echo", "file", "system", "latency", 0}},
    {1, {"lmbench_all", "lat_fs", "/var/tmp", 0}},
    {1, {"busybox", "echo", "Bandwidth", "measurements", 0}},
    {1, {"lmbench_all", "bw_pipe", "-P", "1", 0}},
    {1,
     {"lmbench_all", "bw_file_rd", "-P", "1", "512k", "io_only", "/var/tmp/XXX",
      0}},
    {1,
     {"lmbench_all", "bw_file_rd", "-P", "1", "512k", "open2close",
      "/var/tmp/XXX", 0}},
    {1,
     {"lmbench_all", "bw_mmap_rd", "-P", "1", "512k", "mmap_only",
      "/var/tmp/XXX", 0}},
    {1,
     {"lmbench_all", "bw_mmap_rd", "-P", "1", "512k", "open2close",
      "/var/tmp/XXX", 0}},
    {1, {"busybox", "echo", "context", "switch", "overhead", 0}},
    {1,
     {"lmbench_all", "lat_ctx", "-P", "1", "-s", "32", "2", "4", "8", "16",
      "24", "32", "64", "96", 0}},
    {0, {0, 0}},
};

int main(int argc, char **argv) {
  dev(2, 1, 0);
  dup(0);
  dup(0);
  printf("busybox test\n");
  test_busybox();
  exit(0);
}