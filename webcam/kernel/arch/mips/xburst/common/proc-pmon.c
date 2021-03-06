#include <linux/proc_fs.h>
#include <asm/mipsregs.h>
#include <asm/uaccess.h>
#include <linux/seq_file.h>
#include <jz_proc.h>
#define get_pmon_csr()		__read_32bit_c0_register($16, 7)
#define set_pmon_csr(val)	__write_32bit_c0_register($16, 7, val)

#define get_pmon_high()		__read_32bit_c0_register($16, 4)
#define set_pmon_high(val)	__write_32bit_c0_register($16, 4, val)
#define get_pmon_lc()		__read_32bit_c0_register($16, 5)
#define set_pmon_lc(val)	__write_32bit_c0_register($16, 5, val)
#define get_pmon_rc()		__read_32bit_c0_register($16, 6)
#define set_pmon_rc(val)	__write_32bit_c0_register($16, 6, val)

#define pmon_clear_cnt() do {			\
		set_pmon_high(0);		\
		set_pmon_lc(0);			\
		set_pmon_rc(0);			\
	} while(0)

#define pmon_start() do {			\
		unsigned int csr;		\
		csr = get_pmon_csr();		\
		csr |= 0x100;			\
		set_pmon_csr(csr);		\
	} while(0)
#define pmon_stop() do {			\
		unsigned int csr;		\
		csr = get_pmon_csr();		\
		csr &= ~0x100;			\
		set_pmon_csr(csr);		\
	} while(0)

#define PMON_EVENT_CYCLE 0
#define PMON_EVENT_CACHE 1
#define PMON_EVENT_INST  2
#define PMON_EVENT_TLB   3

#define pmon_prepare(event) do {		\
		unsigned int csr;		\
		pmon_stop();			\
		pmon_clear_cnt();		\
		csr = get_pmon_csr();		\
		csr &= ~0xf000;			\
		csr |= (event)<<12;		\
		set_pmon_csr(csr);		\
	} while(0)

#define BUF_LEN		64
struct pmon_data {
	char buf[BUF_LEN];
};

void on_each_cpu_pmon_prepare(void *info)
{
	pmon_prepare((unsigned int)(info));
}

void on_each_cpu_pmon_start(void *info)
{
	pmon_start();
}

DEFINE_PER_CPU(unsigned int,csr);
DEFINE_PER_CPU(unsigned int,high);
DEFINE_PER_CPU(unsigned int,lc);
DEFINE_PER_CPU(unsigned int,rc);

void on_each_cpu_pmon_read(void *info)
{
	int cpu = smp_processor_id();

	per_cpu(csr, cpu) = get_pmon_csr();
	per_cpu(high, cpu) = get_pmon_high();
	per_cpu(lc, cpu) = get_pmon_lc();
	per_cpu(rc, cpu) = get_pmon_rc();
}

static int pmon_proc_show(struct seq_file *m, void *v)
{
	int cpu,len = 0;
	on_each_cpu(on_each_cpu_pmon_read, NULL, 1);

	for_each_online_cpu(cpu)
	seq_printf(m,"CPU%d:\n%x %x %x %x\n",cpu,
			per_cpu(csr, cpu),
			per_cpu(high, cpu),
			per_cpu(lc, cpu),
			per_cpu(rc, cpu));

	return len;
}


static int pmon_read_proc(struct seq_file *filq, void *v)
{
	pmon_proc_show(filq,NULL);
	return 0;
}



static int pmon_write_proc(struct file *file, const char __user *buffer,
			   size_t count, loff_t *data)
{
	struct pmon_data *d = file->private_data;
	int i;

	if (count > BUF_LEN)
		count = BUF_LEN;
	if (copy_from_user(d->buf, buffer, count))
		return -EFAULT;

	for (i = 0; i < count; ) {
		if (strncmp(&d->buf[i], "cycle", 5) == 0) {
			on_each_cpu(on_each_cpu_pmon_prepare, (void *)PMON_EVENT_CYCLE, 1);
			i += 5 + 1;
		}
		else if (strncmp(&d->buf[i], "cache", 5) == 0) {
			on_each_cpu(on_each_cpu_pmon_prepare, (void *)PMON_EVENT_CACHE, 1);
			i += 5 + 1;
		}
		else if (strncmp(&d->buf[i], "inst", 4) == 0) {
			on_each_cpu(on_each_cpu_pmon_prepare, (void *)PMON_EVENT_INST, 1);
			i += 4 + 1;
		}
		else if (strncmp(&d->buf[i], "tlb", 3) == 0) {
			on_each_cpu(on_each_cpu_pmon_prepare, (void *)PMON_EVENT_TLB, 1);
			i += 3 + 1;
		}
		else if (strncmp(&d->buf[i], "start", 5) == 0) {
			on_each_cpu(on_each_cpu_pmon_start, NULL, 1);
			break;
		}
	}

	return count;
}


static struct jz_single_file_ops gpios_proc_fops ={
	.write = pmon_write_proc,
	.read = pmon_read_proc,
};
static int __init init_proc_pmon(void)
{
	struct proc_dir_entry *jz_proc;
#ifndef CONFIG_USE_JZ_ROOT_DIR
	jz_proc = jz_proc_mkdir("pmon");
#else
	jz_proc = get_jz_proc_root();
#endif
        jz_proc_create("proc-pmon",0644,jz_proc,&gpios_proc_fops);

	return 0;
}

module_init(init_proc_pmon);
