FROM archlinux:base-20201213.0.11146

RUN pacman -Sy make --noconfirm
RUN pacman -Sy gcc --noconfirm
RUN pacman -Sy wget --noconfirm
RUN wget https://github.com/Snaipe/Criterion/releases/download/v2.3.3/criterion-v2.3.3-linux-x86_64.tar.bz2 -O - | tar -xj && \
       cp -r criterion-v2.3.3/include/criterion /usr/include/ && cp criterion-v2.3.3/lib/* /usr/lib/ && \
       cp criterion-v2.3.3/share/pkgconfig/* /usr/share/pkgconfig && \
       rm -rf criterion-v2.3.3 rm -rf criterion-v2.3.3-linux-x86_64.tar.bz

RUN pacman -Sy gdb --noconfirm
RUN pacman -Sy valgrind --noconfirm

RUN pacman -Sy git --noconfirm
RUN pacman -Sy zsh --noconfirm
RUN sh -c "$(wget https://raw.github.com/ohmyzsh/ohmyzsh/master/tools/install.sh -O -)"

RUN pacman -Sy autoconf automake --noconfirm
RUN pacman -Sy autoconf-archive --noconfirm

RUN pacman -Sy openssl --noconfirm
RUN pacman -Sy pkg-config --noconfirm
RUN pacman -Sy libev --noconfirm
RUN pacman -Sy boost --noconfirm

CMD /bin/zsh
