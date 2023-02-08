# C filter to make RTF file more readable

### example in .vimrc:

```
autocmd BufReadPre *.rtf set filetype=rtf
autocmd BufReadPre *.rtf set fenc=utf8
autocmd BufReadPost *.rtf :%!utf8rtf --decode
autocmd BufWritePre *.rtf :%!utf8rtf --encode
autocmd BufWritePre *.rtf set fenc=cp1251
```
