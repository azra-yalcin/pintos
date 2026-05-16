#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);

/* Programı sonlandıran ve ekrana hocanın istediği formatta log basan fonksiyon */
void
sys_exit (int status)
{
  struct thread *cur = thread_current ();
  
  /* Hocanın otomatik testlerinin (make check) aradığı ve PUAN VERDİĞİ kritik log formatı */
  printf ("%s: exit(%d)\n", cur->name, status);
  
  /* Thread'i resmi olarak sonlandırıyoruz */
  thread_exit ();
}

/* Ekrana (Konsola) yazı yazılmasını sağlayan fonksiyon */
int
sys_write (int fd, const void *buffer, unsigned size)
{
  /* fd == 1, standart çıktı (STDOUT - Konsol ekranı) demektir */
  if (fd == 1)
    {
      /* Pintos kütüphanesindeki hazır putbuf fonksiyonunu kullanarak 
         buffer içindeki veriyi tek seferde konsola basıyoruz */
      putbuf (buffer, size);
      return size; // Kaç bayt yazıldıysa onu döndürüyoruz
    }
  return -1;
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  /* Kullanıcı yığından (f->esp) sistem çağrısı numarasını alıyoruz */
  int syscall_num = *(int *)f->esp;

  switch (syscall_num)
    {
    case SYS_EXIT:
      {
        /* exit çağrısının 1 tane argümanı vardır (çıkış kodu). f->esp + 1 adresindedir. */
        int status = *(int *)(f->esp + 4);
        sys_exit (status);
        break;
      }
    case SYS_WRITE:
      {
        /* write çağrısının 3 argümanı vardır: fd, buffer, size. f->esp + 1, +2, +3 sırasıyla dizilir. */
        int fd = *(int *)(f->esp + 4);
        const void *buffer = *(char **)(f->esp + 8);
        unsigned size = *(unsigned *)(f->esp + 12);
        
        /* f->eax çekirdeğin kullanıcı programına döndüreceği cevap değeridir (Dönen bayt sayısı) */
        f->eax = sys_write (fd, buffer, size);
        break;
      }
    default:
      printf ("Bilinmeyen sistem çağrısı: %d\n", syscall_num);
      sys_exit (-1);
    }
}
