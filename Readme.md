# C filter to make RTF file more readable

```
Привет МИР 
```
instead of 

```
\u1055 \u1088 \u1080 \u1074 \u1077 \u1090  \u1052 \u1048 \u1056
```

### example in .vimrc:

```
autocmd BufReadPre *.rtf set filetype=rtf
autocmd BufReadPre *.rtf set fenc=utf8
autocmd BufReadPost *.rtf :%!utf8rtf --decode
autocmd BufWritePre *.rtf :%!utf8rtf --encode
autocmd BufWritePre *.rtf set fenc=cp1251
```
