FLAGS="-O3 -fomit-frame-pointer -march=core2"

rm -rf *.o *.c~ *.lo *.so *.a .libs thomson-*
echo; echo; echo "Build against libgcrypt"; echo
gcc $FLAGS -pthread -static -DLIBGCRYPT -o thomson-gcrypt.a -c thomson.c
gcc $FLAGS -pthread -static -o thomson-gcrypt thomson-gcrypt.a /lib/libgcrypt.a /lib/libgpg-error.a
strip thomson-gcrypt

echo; echo; echo "Build against libcrypto (openssl)"; echo
gcc $FLAGS -pthread -static -DLIBCRYPTO -o thomson-openssl.a -c thomson.c
gcc $FLAGS -pthread -static -o thomson-openssl thomson-openssl.a /usr/lib/libcrypto.a
strip thomson-openssl

echo; echo; echo "Build against libtomcrypt"; echo
gcc $FLAGS -pthread -static -DLIBTOMCRYPT -o thomson-tomcrypt.a -c thomson.c
gcc $FLAGS -pthread -static -o thomson-tomcrypt thomson-tomcrypt.a /usr/lib/libtomcrypt.a
strip thomson-tomcrypt
