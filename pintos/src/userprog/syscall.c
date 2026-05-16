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

#include "threads/vaddr.h" /* Üstte yoksa kesinlikle ekle */

/* Adresin kullanıcı alanında ve geçerli olup olmadığını kontrol eden yardımcı fonksiyon */
static void
check_valid_ptr (const void *vaddr)
{
  /* 1. İşaretçi NULL mı?
     2. İşaretçi çekirdek (Kernel) alanını mı gösteriyor? (is_user_vaddr kontrolü)
     3. Bu adres sayfa tablosunda haritalanmış mı? */
  if (vaddr == NULL || !is_user_vaddr (vaddr) || pagedir_get_page (thread_current ()->pagedir, vaddr) == NULL)
    {
      sys_exit (-1); /* Eğer hileli/kötü bir adres ise işlemi derhal -1 ile öldür */
    }
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  /* ÖNEMLİ: Önce f->esp adresinin kendisi güvenli mi diye bakıyoruz */
  check_valid_ptr (f->esp);

  int syscall_num = *(int *)f->esp;

  switch (syscall_num)
    {
    case SYS_EXIT:
      {
        check_valid_ptr (f->esp + 4);
        int status = *(int *)(f->esp + 4);
        sys_exit (status);
        break;
      }
    case SYS_WRITE:
      {
        check_valid_ptr (f->esp + 4);
        check_valid_ptr (f->esp + 8);
        check_valid_ptr (f->esp + 12);

        int fd = *(int *)(f->esp + 4);
        const void *buffer = *(char **)(f->esp + 8);
        unsigned size = *(unsigned *)(f->esp + 12);
        
        /* Buffer'ın işaret ettiği string'in içeriği de güvenli mi kontrolü */
        check_valid_ptr (buffer);

        f->eax = sys_write (fd, buffer, size);
        break;
      }
    default:
      sys_exit (-1);
    }
}