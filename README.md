# База данных микросхем Gnome client

```
cd ~/git
git clone git@github.com:commandus/rcr.git
git clone git@github.com:commandus/rcr-gnome.git
```

Слинковать
```
cd ~/git/rcr-gnome/third-party
ln -s ~/git/rcr rcr
ls -la
lrwxrwxrwx  1 andrei andrei    20 апр 28 12:17 rcr -> /home/andrei/git/rcr
```

```
echo $XDG_DATA_DIRS
/usr/share/ubuntu:/usr/local/share/:/usr/share/:/var/lib/snapd/desktop
```

## Third party libraries

[The International Components for Unicode (ICU) libraries](https://icu.unicode.org/)

[xlnt library for manipulating spreadsheets](https://github.com/tfussell/xlnt)

[gRPC high performance, open source universal RPC framework](https://grpc.io/)

## Acknowledgements 

$XDG_DATA_DIRS
/usr/share/ubuntu:/usr/local/share/:/usr/share/:/var/lib/snapd/desktop

Logo [Miroku Sama](https://thenounproject.com/icon/stack-24435/) 

[Install gtkmm](https://gist.github.com/jerrywang94/ffd370d2e42918817bbfb765def7d771)


## Скопировать файлы локализации

```
tools/update-translation.sh 
sudo cp locale/ru/LC_MESSAGES/*.mo /usr/share/locale/ru/LC_MESSAGES/```
```
