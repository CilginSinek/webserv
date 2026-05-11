# TCP Server Socket ve accept() Mantığı

## 1. Önce socket oluşturulur

```cpp
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
```

Bu sadece bir file descriptor (FD) oluşturur.

Henüz kimseyi dinlemiyor.

---

# 2. bind()

```cpp
bind(server_fd, ...);
```

Burada diyorsun ki:

> “Bu socket şu IP:PORT’a bağlı.”

Örneğin:

```text
0.0.0.0:8080
```

---

# 3. listen()

En kritik nokta.

```cpp
listen(server_fd, SOMAXCONN);
```

Bundan sonra socket:

# LISTENING SOCKET

olur.

Yani kernel’e diyorsun ki:

> “Bu porta gelen TCP connection isteklerini kabul et.”

---

# Client geldiğinde ne oluyor?

Browser:

```text
localhost:8080
```

adresine bağlanmak ister.

TCP handshake başlar.

---

# TCP 3-Way Handshake

Client:

```text
SYN
```

gönderir.

Kernel bunu görür.

Çünkü:

OS zaten tüm network paketlerini sürekli dinliyor.

```text
Network Card
↓
Kernel TCP Stack
```

---

# Kernel ne kontrol ediyor?

Kernel şuna bakar:

> “Bu paket hangi port için geldi?”

Örneğin:

```text
PORT 8080
```

Sonra kontrol eder:

> “8080 portunda listening socket var mı?”

Evet:

```text
server_fd = 3
```

---

# Sonra kernel ne yapar?

Connection request’i:

# listen backlog queue

içine koyar.

---

# ÇOK ÖNEMLİ

`listen()` aslında gizlice bir queue oluşturur.

Örneğin:

```cpp
listen(fd, 128);
```

demek:

> “128 tane bekleyen bağlantı tut.”

Şu anda henüz `accept()` yok.

Sadece kernel diyor ki:

> “Yeni client bağlanmak istiyor.”

---

# Peki server bunu nasıl anlıyor?

İşte burada:

- epoll
- select
- kqueue

devreye giriyor.

Server:

```cpp
epoll_wait(...)
```

der.

Kernel’e şunu sorar:

> “Hangi fd’lerde event var söyle.”

---

# Kernel cevap verir

```text
fd 3 readable
```

---

# BURASI ÇOK KRİTİK

Listening socket’in:

```text
readable
```

olması ne demek?

Bu:

# “accept queue içinde bekleyen client var”

demektir.

Yani:

Listening socket için:

```text
readable != veri geldi
```

demek değildir.

Listening socket için:

```text
readable = yeni connection geldi
```

demektir.

---

# Sonra ne olur?

Server:

```cpp
accept(server_fd);
```

çağırır.

---

# accept() ne yapıyor?

Kernel backlog queue’dan:

bir client alır.

Ve:

# YENİ bir socket oluşturur

Örneğin:

```text
server_fd = 3
client_fd = 42
```

---

# ÇOK KRİTİK

Listening socket:

```text
3
```

hala yaşamaya devam eder.

Yeni clientlar için.

---

# client_fd ne?

Artık o kullanıcıyla özel iletişim hattı.

Bundan sonra:

```cpp
recv(client_fd, ...)
send(client_fd, ...)
```

bunun üstünden yapılır.

---

# Görsel Akış

## Başlangıç

```text
server_fd = 3
LISTENING
```

---

## Client geliyor

```text
Chrome → localhost:8080
```

---

## Kernel

Connection request’i queue’ya koyar.

```text
backlog:
[client1]
```

---

## epoll_wait döner

```text
fd 3 readable
```

---

## accept()

```cpp
client_fd = accept(3);
```

---

# Sonuç

```text
server_fd = 3  -> hala listening
client_fd = 42 -> gerçek client socket
```

---

# Bundan sonra

Artık:

```cpp
recv(42);
send(42);
```

ile gerçek HTTP verisi okunur/yazılır.

---

# Neden accept() ayrı?

Çünkü:

## Listening socket

```text
Sadece kapı
```

---

## Client socket

```text
Gerçek konuşma kanalı
```

---

# Gerçek Dünya Analojisi

## listen socket

Apartman kapısı.

---

## Kernel backlog queue

Kapıda bekleyen insanlar kuyruğu.

---

## accept()

Bir kişiyi içeri almak.

---

## client_fd

O kişiye özel oda açılması.

---

# Özet

## listen()

Kernel’e:

> “Bu portu dinle.”

der.

---

# Client geldiğinde

Kernel:

1. TCP handshake yapar
2. Connection request’i queue’ya koyar

---

# epoll/select

Server’a der ki:

> “Listening socket hazır.”

---

# accept()

Queue’dan client alır.

Yeni client socket oluşturur.

---

# Bundan sonra

Artık:

```cpp
recv(client_fd);
```

ile gerçek HTTP verisi okunur.