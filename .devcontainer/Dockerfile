FROM archlinux:base-20210214.0.15477

RUN patched_glibc=glibc-linux4-2.33-4-x86_64.pkg.tar.zst && \
       curl -LO "https://repo.archlinuxcn.org/x86_64/$patched_glibc" && \
       bsdtar -C / -xvf "$patched_glibc"

RUN pacman -Sy --noconfirm make gcc wget gdb valgrind git zsh autoconf automake autoconf-archive openssl pkg-config libev boost gnu-netcat
RUN wget https://github.com/Snaipe/Criterion/releases/download/v2.3.3/criterion-v2.3.3-linux-x86_64.tar.bz2 -O - | tar -xj && \
       cp -r criterion-v2.3.3/include/criterion /usr/include/ && cp criterion-v2.3.3/lib/* /usr/lib/ && \
       cp criterion-v2.3.3/share/pkgconfig/* /usr/share/pkgconfig && \
       rm -rf criterion-v2.3.3 rm -rf criterion-v2.3.3-linux-x86_64.tar.bz

RUN sh -c "$(wget https://raw.github.com/ohmyzsh/ohmyzsh/master/tools/install.sh -O -)"

RUN pacman -Sy --noconfirm openssl mkcert python-pip
RUN pip install requests
RUN pacman -Sy --noconfirm ccache
RUN pacman -Sy --noconfirm clang

RUN echo "alias configure=\"CXX='ccache g++' ./configure\"" >> ~/.zshrc

CMD /bin/zsh
