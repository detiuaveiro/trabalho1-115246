[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/wH6E8Dzd)
# Trabalho1 AED 2023-2024

Este é o código-fonte para o primeiro projeto de AED 2023-2024.

## Ficheiros

- `image8bit.c` - implementação do módulo (a COMPLETAR)
- `image8bit.h` - interface do módulo
- `instrumentation.[ch]` - módulo para contagens de operações e medição de tempos
- `imageTest.c` - programa de teste simples
- `imageTool.c` - programa de teste mais versátil
- `Makefile` - regras para compilar e testar usando `make`

- `README.md` - estas informações que está a ler
- `Design-by-Contract.md` - explicação sobre [metodologia DbC][dbc],
   seguida neste projeto.


[dbc]: Design-by-Contract.md

## Descarregar imagens

Execute:

- `make pgm` - para descarregar imagens para pasta `pgm/`
- `make setup` - para descarregar imagens para testes em `test/`

## Compilar

- `make` - Compila e gera os programas de teste.
- `make clean` - Limpa ficheiros objeto e executáveis.


## Sugestões para o desenvolvimento

Sugere-se o desenvolvimento progressivo pela seguinte ordem:

1. Completar `ImageCreate`-LISTO- e `ImageDestroy`-LISTO-.
2. Compilar os programas correndo `make`
   e testar com

   ```bash
   valgrind ./imageTool test/original.pgm save out.pgm`
   ```
   
   Isto chama `ImageLoad`, que chama `ImageCreate`
   e depois `ImageSave` e `ImageDestroy`.
   Verificar que toda a memória foi libertada.-LISTO-
3. Completar a função interna `G`-LISTO-,
   que é usada por `ImageSetPixel` e `ImageGePixel`.
4. Completar `ImageStats`.-LISTO-
   Se usar `ImageGePixel`, permitirá testar `G`-LISTO-.
   Testar com `./imageTool test/original.pgm info`.-LISTO-
5. Completar`ImageNegative`-LISTO-VERIFICADO, 1
            `ImageThreshold`-LISTO-VERIFICADO, 2
            `ImageBrighten`-LISTO-VERIFICADO.3
6. Completar `ImageValidRect`.-LISTO-VERIFICADO
7. Completar `ImageMirror`-LISTO-VERIFICADO, 4
            `ImageRotate`.-LISTO-VERIFICADO  5
8. Completar `ImageCrop`LISTO-VERIFICADO,   6
            `ImagePaste`LISTO- VERFICADO e  7
            `ImageBlend`.LISTO-VERIFICADO  8
9. Completar `ImageMatchSubImage`.LISTO- e  
            `ImageLocateSubImage`..LISTO- 
10. Completar `ImageBlur`..LISTO-VERIFICADO 9

Pode executar `make test1`, `make test2`, etc.
para fazer testes simples a muitas destas funções.
Mas faça outros testes que considere adequados.

## Atualizar repositório


Dada a natureza do trabalho, poderá ser necessário
atualizar repositório base (upstream) deste projeto.
Se isso acontecer, deverá atualizar o seu repositório com os seguintes comandos:

```bash
git remote add upstream git@github.com:detiuaveiro/image8bit-pub.git
git fetch upstream
git rebase upstream/main
```


