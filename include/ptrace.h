const
int ptrace_write(
		const pid_t,
		const void *const,
		const uint8_t *const,
		size_t);

const
int ptrace_read(
		const pid_t,
		const void *const,
		void *const,
		const size_t);

void ptrace_reset_amd64(
		const pid_t child_pid,
		const unsigned long start);

void ptrace_reset_x86(
		const pid_t child_pid,
		const unsigned long start);

void ptrace_reset_arm(
		const pid_t child_pid,
		const unsigned long start);

void ptrace_child(
		const int);

void ptrace_launch(
		const pid_t);

const
int ptrace_reap(
		const pid_t,
		struct proc_info_t *const);

void ptrace_cont(const pid_t,
		struct proc_info_t *const);

void ptrace_detatch(
		const pid_t child_pid,
		struct proc_info_t *const);
