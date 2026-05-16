---- GRUP ----

>> Grup üyelerinizin adlarını ve e-posta adreslerini doldurun.

Adı Soyadı <email@ogr.deu.edu.tr>  
Adı Soyadı <email@ogr.deu.edu.tr>  
Adı Soyadı <email@ogr.deu.edu.tr>

---- ÖN HAZIRLIKLAR ----

>> Teslimatınızla ilgili herhangi bir ön hazırlık yorumunuz varsa lütfen burada belirtin.

Yok.

>> Pintos dokümantasyonu, ders kitabı, ders notları dışında, teslimatınızı hazırlarken başvurduğunuz çevrimdışı veya çevrimiçi kaynakları belirtiniz.

Yok.

                 ALARM SAATİ
                 ===========

---- VERİ YAPILARI ----

>> A1: Her yeni veya değiştirilen `struct` veya `struct` üyesi, global veya statik değişken,
>> `typedef` veya enumerasyonun beyanını buraya kopyalayın. Her birinin amacını 25 kelime
>> veya daha az bir şekilde tanımlayın.

`threads/thread.h` içinde `struct thread`'e eklenen alan:

```c
int64_t wake_tick;   /* Thread'in uyandırılması gereken timer tick değeri. */
```

`devices/timer.c` içine eklenen global değişken:

```c
static struct list sleep_list;
/* Uyku modundaki (bloklanmış) thread'lerin sıralı listesi;
   wake_tick değerine göre küçükten büyüğe sıralıdır. */
```

---- ALGORİTMALAR ----

>> A2: timer_sleep() fonksiyonuna yapılan bir çağrıda neler olduğunu,
>> zamanlayıcı kesme işleyicisinin etkilerini kısaca açıklayın.

`timer_sleep(ticks)` çağrıldığında:
1. Çağıran thread'in `wake_tick` değeri `timer_ticks() + ticks` olarak ayarlanır.
2. Kesmeler devre dışı bırakılır.
3. Thread, `wake_tick` değerine göre sıralı biçimde `sleep_list`'e eklenir.
4. `thread_block()` çağrılarak thread bloklanır; CPU başka bir thread'e verilir.
5. Kesmeler yeniden etkinleştirilir.

Timer kesme işleyicisi (`timer_interrupt`) her tick'te:
1. `sleep_list`'in başındaki thread'in `wake_tick` değerini kontrol eder.
2. `timer_ticks() >= wake_tick` koşulu sağlanan tüm thread'leri listeden çıkarır
   ve `thread_unblock()` ile hazır kuyruğuna ekler.
3. Liste sıralı olduğundan, ilk koşulu sağlamayan thread'e ulaşınca döngü sonlanır.

>> A3: Zamanlayıcı kesme işleyicisi içinde harcanan süreyi minimize etmek için
>> hangi adımlar atılmaktadır?

- `sleep_list`, `wake_tick` değerine göre küçükten büyüğe sıralı tutulur.
  Böylece kesme işleyicisi listenin tamamını taramak yerine yalnızca ön kısma bakar.
- `wake_tick >= timer_ticks()` olan ilk thread'e ulaşıldığında döngü hemen sonlanır.
- Listeye ekleme işlemi `timer_sleep()` içinde (kesme bağlamı dışında) yapılır;
  kesme işleyicisi yalnızca okuma ve çıkarma yapar.

---- SENKRONİZASYON ----

>> A4: Birden fazla iş parçacığı aynı anda timer_sleep() fonksiyonunu çağırdığında
>> yarış koşulları nasıl engellenir?

`timer_sleep()` içinde `sleep_list`'e erişim öncesinde `intr_disable()` ile kesmeler
devre dışı bırakılır. Bu sayede hem diğer thread'lerin hem de timer kesme işleyicisinin
listeye eş zamanlı erişimi engellenir. `thread_block()` çağrısı da kesmeler kapalıyken
yapıldığından thread'in bloklanması atomik olarak gerçekleşir.

>> A5: timer_sleep() fonksiyonu çağrıldığında bir zamanlayıcı kesmesi meydana gelirse
>> yarış koşulları nasıl engellenir?

`sleep_list`'e ekleme ve `thread_block()` çağrısı, kesmeler devre dışıyken yapılır.
Dolayısıyla bu kritik bölge çalışırken timer kesmesi tetiklenemez. `wake_tick`
ayarlandıktan sonra ancak thread bloklanmadan önce bir kesme gelse bile; kesme,
henüz `sleep_list`'e eklenmemiş bu thread'i göremez ve thread uyanma tick'ini
kaçırmaz — `timer_sleep()` yeniden `thread_block()`'a kadar devam eder.

---- GEREKÇE ----

>> A6: Bu tasarımı neden seçtiniz? Diğer düşündüğünüz tasarımlara kıyasla hangi
>> açılardan daha üstün olduğunu düşünüyorsunuz?

Seçilen tasarım (sıralı uyku listesi + thread bloklaması), orijinal busy-waiting
yaklaşımına kıyasla CPU'yu verimli kullanır: uyuyan thread'ler zamanlanmaz.

Sırasız bir liste tutmak da mümkün olurdu; ancak sıralı liste sayesinde kesme
işleyicisi O(1) (ya da az sayıda uyanma varsa O(k)) karmaşıklıkla çalışır;
listeyi her seferinde baştan sona taramak gerekmez. Ekleme O(n) olmakla birlikte,
bu işlem kesme bağlamı dışında yapıldığından performans etkisi kabul edilebilirdir.

Bir semafor veya koşul değişkeni kullanmak da düşünülebilirdi; ancak bu yapılar
ek karmaşıklık getirir ve timer_sleep için gereken tek seferlik uyandırma semantiğine
doğrudan `thread_block`/`thread_unblock` çifti daha iyi uyar.

             ÖNCELİKLE PLANLAMA
             ===================

---- VERİ YAPILARI ----

>> B1: Her yeni veya değiştirilen `struct` veya `struct` üyesi, global veya statik değişken,
>> `typedef` veya enumerasyonun beyanını buraya kopyalayın. Her birinin amacını 25 kelime
>> veya daha az bir şekilde tanımlayın.

`threads/thread.h` içinde `struct thread`'e eklenen alanlar:

```c
int base_priority;
/* Öncelik bağışından bağımsız, thread'in özgün (taban) önceliği. */

struct list locks_held;
/* Thread'in şu an elinde tuttuğu kilitlerin listesi; bağış hesabında kullanılır. */

struct lock *lock_waiting;
/* Thread'in beklediği kilit; iç içe bağış zinciri için kullanılır. NULL ise beklemiyordur. */
```

`threads/synch.h` içinde `struct lock`'a eklenen alanlar:

```c
struct list_elem elem;
/* `locks_held` listesine bağlamak için. */

int max_priority;
/* Bu kilidi bekleyen thread'ler arasındaki en yüksek öncelik; bağış takibi için. */
```

>> B2: Öncelik bağışını izlemek için kullanılan veri yapısını açıklayın.
>> Bir iç içe bağışı ASCII sanatı ile diyagramlayın.

Her thread, elinde tuttuğu kilitlerin listesini (`locks_held`) ve beklediği kilidi
(`lock_waiting`) tutar. Her kilit, bekleyenler arasındaki en yüksek önceliği
(`max_priority`) saklar. Bir thread'in efektif önceliği, `base_priority` ile
`locks_held` listesindeki tüm kilitlerin `max_priority` değerlerinin maksimumu
arasından büyük olandır.

İç içe bağış örneği (H önceliği 33, M önceliği 32, L önceliği 31):

```
H (pri=33) --> lock_B --> M (pri=32, efektif=33)
                               |
                               v
                           lock_A --> L (pri=31, efektif=33)

Zincir:
  H, lock_B için bekler  →  M'ye 33 bağışlanır
  M, lock_A için bekler  →  L'ye 33 bağışlanır (iç içe bağış)
```

---- ALGORİTMALAR ----

>> B3: Bir kilit, semafor veya koşul değişkeni için bekleyen en yüksek öncelikli
>> iş parçacığının ilk önce uyanmasını nasıl sağlarsınız?

`sema_up()` ve `cond_signal()` fonksiyonlarında bekleme listesi (`waiters`) her
uyandırma işleminden önce önceliğe göre sıralanır (`list_sort` ile). Böylece
listenin başındaki her zaman o anki en yüksek öncelikli thread'i gösterir.
Öncelikler dinamik olarak değişebildiğinden (bağış nedeniyle), ekleme sırasında
sıralı tutmak yerine `sema_up` anında tekrar sıralama daha güvenlidir.

>> B4: lock_acquire() fonksiyonu bir öncelik bağışı oluşturduğunda olaylar sırasını
>> açıklayın. İç içe bağış nasıl işlenir?

1. Thread kilidi almaya çalışır; kilit başka bir thread'deyse:
2. Çağıran thread `lock->max_priority`'yi kendi efektif önceliği ile günceller.
3. `lock->holder`'ın önceliği, `lock->max_priority`'den düşükse yükseltilir.
4. İç içe bağış için: `lock->holder->lock_waiting` işaret ettiği kilide gidilir,
   aynı güncelleme o kilidin sahibine de uygulanır. Bu zincir, NULL'a ya da
   güncelleme gereksiz olana dek (en fazla 8 adım) sürer.
5. Thread `sema_down` ile bloklanır; `lock_waiting` alanı bu kilide ayarlanır.
6. Kilit alındığında kilit `locks_held` listesine eklenir, `lock_waiting` NULL yapılır.

>> B5: lock_release() fonksiyonu, yüksek öncelikli bir iş parçacığının beklediği
>> bir kilit üzerinde çağrıldığında olaylar sırasını açıklayın.

1. Kilit `locks_held` listesinden çıkarılır.
2. Thread'in efektif önceliği yeniden hesaplanır: kalan kilitlerin `max_priority`
   değerlerinin maksimumu ile `base_priority` karşılaştırılır; büyük olan atanır.
3. `sema_up()` çağrılır: bekleme listesi sıralanır ve en yüksek öncelikli thread
   unblock edilir.
4. Yeni unblock edilen thread mevcut thread'den yüksek önceliğe sahipse
   `thread_yield()` tetiklenir ve CPU o thread'e geçer.

---- SENKRONİZASYON ----

>> B6: thread_set_priority() fonksiyonunda potansiyel bir yarış koşulunu açıklayın
>> ve uygulamanızın bunu nasıl engellediğini açıklayın. Bu yarış koşulunu engellemek
>> için bir kilit kullanabilir misiniz?

Yarış koşulu: Bir thread `base_priority`'yi güncellerken, timer kesmesi aynı thread'in
önceliğini okuyan `thread_get_priority()` ya da zamanlayıcıyı çağırabilir. Bu durumda
tutarsız öncelik değeri okunabilir.

Engelleme: `thread_set_priority()` içinde `intr_disable()` / `intr_set_level()` ile
kesmeler geçici olarak devre dışı bırakılır; böylece güncelleme atomik hale gelir.

Kilit kullanılabilir mi? Hayır. `thread_set_priority()` bir kesme işleyicisinden
(dolaylı olarak) çağrılabilir ve kesmeler bağlamında kilit alınamaz (kilitler
`thread_block` kullanır, kesme bağlamında uyumak yasaktır). Bu nedenle kilit yerine
kesme devre dışı bırakma tercih edilmelidir.

---- GEREKÇE ----

>> B7: Bu tasarımı neden seçtiniz? Diğer düşündüğünüz tasarımlara kıyasla hangi
>> açılardan daha üstün olduğunu düşünüyorsunuz?

Kilit bazlı bağış takibi (her thread `locks_held`, her kilit `max_priority`),
en basit ve doğrudan yaklaşımdır. Alternatif olarak her thread'de tüm bağışçıların
listesi tutulabilirdi; ancak bu, kilit serbest bırakıldığında bu listenin
temizlenmesini gerektirir ve daha karmaşık bir yapıya yol açar.

Mevcut tasarımın avantajları:
- Bağış zinciri (`lock_waiting` işaretçisi) sayesinde iç içe bağış O(derinlik) ile çözülür.
- `max_priority` her kilitte tutulduğundan, kilit bırakıldığında thread önceliği
  O(tutulan kilit sayısı) ile yeniden hesaplanır; tüm bağışçı listesi gerekmez.
- Kod yapısı anlaşılır ve hata ayıklaması kolaydır.

              GELİŞMİŞ PLANLAMA
              ==================

---- VERİ YAPILARI ----

>> C1: Her yeni veya değiştirilen `struct` veya `struct` üyesi, global veya statik değişken,
>> `typedef` veya enumerasyonun beyanını buraya kopyalayın. Her birinin amacını 25 kelime
>> veya daha az bir şekilde tanımlayın.

`threads/thread.h` içinde `struct thread`'e eklenen alanlar:

```c
int nice;
/* Thread'in nice değeri [-20, 20]; yüksek nice CPU'yu başkalarına bırakır. */

fixed_point_t recent_cpu;
/* Son zamanlarda kullanılan CPU süresinin üstel ağırlıklı ortalaması (sabit noktalı). */
```

`threads/thread.c` içine eklenen global değişkenler:

```c
static fixed_point_t load_avg;
/* Sistem genelindeki ortalama hazır thread sayısı (sabit noktalı, üstel ortalama). */
```

`threads/fixed-point.h` — yeni dosya:

```c
typedef int fixed_point_t;
/* 17.14 sabit noktalı sayı temsili için typedef; tam sayı kısmı 17 bit, kesir 14 bit. */
```

---- ALGORİTMALAR ----

>> C2: A, B ve C iş parçacıklarının nice değerleri sırasıyla 0, 1 ve 2.
>> Her birinin recent_cpu değeri 0. Aşağıdaki tabloyu doldurun.

Formüller (4.4BSD):
- priority = PRI_MAX - (recent_cpu / 4) - (nice * 2)
- recent_cpu her tick çalışan thread için +1 artar
- Her 4 tick'te öncelikler yeniden hesaplanır
- Her saniyede (100 tick) recent_cpu ve load_avg güncellenir

Başlangıç değerleri: A(nice=0, rcpu=0, pri=63), B(nice=1, rcpu=0, pri=61), C(nice=2, rcpu=0, pri=59)

```
timer  recent_cpu      priority     thread
ticks   A    B    C    A    B    C  to run
-----  ---  ---  ---  ---  ---  ---  ------
 0      0    0    0   63   61   59     A
 4      4    0    0   62   61   59     A
 8      8    0    0   61   61   59     B
12      8    4    0   61   60   59     A
16     12    4    0   60   60   59     B
20     12    8    0   60   59   59     A
24     16    8    0   59   59   59     C
28     16    8    4   59   59   58     B
32     16   12    4   59   58   58     A
36     20   12    4   58   58   58     C
```

>> C3: Planlayıcı spesifikasyonundaki herhangi bir belirsizlik tablodaki değerleri
>> belirsiz hale getirdi mi? Eğer öyleyse, bunları çözmek için hangi kuralı kullandınız?

Evet, iki belirsizlik bulunmaktadır:

1. Tick 0'da öncelikler eşit olmadığı için sorun yok; ancak ilerleyen tick'lerde
   birden fazla thread aynı önceliğe ulaştığında hangisi çalışacak? Kural olarak,
   en uzun süredir çalışmayan thread tercih edilir (round-robin); tabloda bu durum
   öncelikler ilk eşitlendiğinde round-robin sırası gözetilerek yansıtılmıştır.

2. Öncelik yeniden hesaplaması, çalışan thread yeni tick'te artış almadan önce mi
   sonra mı yapılır? Spesifikasyona göre recent_cpu önce artar, ardından her 4
   tick'te öncelikler yeniden hesaplanır; tablo bu sıraya göre doldurulmuştur.

Bu kurallar, uygulamadaki davranışla tutarlıdır.

>> C4: Planlamayı kod içinde ve kesme bağlamı dışında yapılan işlerin maliyeti
>> arasında nasıl böldüğünüz, performansı nasıl etkiler?

Kesme bağlamında (timer_interrupt içinde) yapılanlar:
- Çalışan thread'in recent_cpu değerini +1 artırma (O(1)).
- Her 4 tick'te tüm thread'lerin önceliğini yeniden hesaplama (O(n)).
- Her saniyede load_avg ve tüm thread'lerin recent_cpu değerini güncelleme (O(n)).

Kesme bağlamı dışında yapılanlar:
- thread_set_nice() çağrısı sonrası öncelik güncellemesi ve gerekirse yield.

O(n) işlemler kesme bağlamında yapıldığından, thread sayısı arttıkça kesme işleme
süresi uzar. Bu, diğer thread'lerin tick sürelerini çalar ve ölçüm hatalarına
neden olabilir. Daha büyük sistemlerde bu hesaplamaların bir kısmının arka plan
thread'lerine taşınması düşünülebilir; ancak bu projenin kapsamında O(n) kabul
edilebilirdir.

---- GEREKÇE ----

>> C5: Tasarımınızı kısaca eleştirin, tasarım seçimlerinizdeki avantajları ve
>> dezavantajları belirtin.

Avantajlar:
- 17.14 sabit noktalı aritmetik, çekirdek içinde kayan nokta kullanımını önler;
  x86 çekirdeğinde FPU durumu thread başına kaydedilemeyeceğinden bu zorunludur.
- `fixed-point.h` makro/fonksiyon soyutlaması, tüm hesaplamaları tek bir yerde toplar
  ve hata ayıklamayı kolaylaştırır.
- Tek bir `load_avg` global değişkeni yeterlidir; karmaşık veri yapısı gerekmez.

Dezavantajlar:
- Her 4 tick'te O(n) öncelik güncellemesi ölçeklenebilir değildir; 64 öncelik kuyruğu
  kullanılsa bile hesaplama maliyeti kaçınılmazdır.
- Sabit noktalı aritmetik hataları sessizce yutabilir; yanlış bit kaydırma tüm
  zamanlayıcı davranışını bozar.

İyileştirme önerisi: Güncellemeleri batch halinde yapmak yerine, thread hazır kuyruğuna
girdiğinde recent_cpu'yu tembel (lazy) hesaplamak; bu, yüksek thread sayılarında
kesme süresini önemli ölçüde azaltır.

>> C6: Sabit nokta matematiği uygulamanız hakkında kararınızı açıklayın.

`threads/fixed-point.h` adında ayrı bir başlık dosyası oluşturuldu. İçinde:
- `fixed_point_t` typedef'i (int üzerinde 17.14 formatı)
- Dönüşüm, toplama, çıkarma, çarpma ve bölme için `static inline` fonksiyonlar
  ya da makrolar

Bu soyutlama katmanının nedenleri:
1. Sabit noktalı çarpma/bölmede doğru bit kaydırma miktarı kritiktir; bunu her
   kullanım yerinde tekrar yazmak hata riskini artırır.
2. Fonksiyon isimleri (örn. `fp_mul_int`, `fp_div_fp`) kodun okunabilirliğini artırır;
   ham bit işlemleri kodu anlaşılmaz kılar.
3. Format ileride değiştirilmek istenirse (örn. 17.14 → 16.16) yalnızca bu dosya
   güncellenir.

Makro yerine `static inline` fonksiyon tercih edildi; bu sayede tip kontrolü yapılır
ve hata ayıklamada semboller görünür olur.
