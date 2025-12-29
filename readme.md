This is a modified version of CGE7 by **[Youkan700](https://github.com/SHARPENTIERS/CGE7)**, the screen editor for Sharp MZ700. The modifications are:

- Added Visual Studio 2022 project (also works with Jetbrains Rider)
- Translated UI
- Modified String input to allow lowercase with non Japanese character set
- Removed Japanese string input
- Added `2nd Character Set` checkbox in string input dialog to use 2nd charset upper case characters.

All these modifications require the international MZ 700 font ROM (can be found **[here](https://original.sharpmz.org/download/mz7cgint.zip)** - Rename the file as `MZ700FON.DAT` and put it in the same folder as the exe file).

- Added an `Export` option in the `File` menu. This option exports the screen as an assembly source. There are two export options: `Raw` and `Compressed`. `Raw` saves the screen as is, `Compressed` saves the screen with a RLE compression. This file also contains the animation blocks.

Here is the code to display the screen content:


##### Raw

    DISPLAY_SCREEN:
        LD      HL,SCREEN_CHARS
        LD      DE,$D000
        LD      BC,1000
        LDIR
        LD      HL,SCREEN_ATTRIBUTES
        LD      DE,$D800
        LD      BC,1000
        LDIR
        RTS

#### Compressed

    UNCOMPRESS:
        LD      B,(IX)
        INC     IX
        LD      A,(IX)
        INC     IX
        CP      0
        JR      Z,.END
    .LOOP:
        LD      (HL),B
        INC     HL
        DEC     A
        JR      NZ,.LOOP
        JR      UNCOMPRESS
    .END:
        RET

    DISPLAY_COMPRESSED_SCREEN:
        LD      IX,SCREEN_CHARS
        LD      HL,$D000
        CALL    UNCOMPRESS
        LD      IX,SCREEN_ATTRIBUTES
        LD      HL,$D800
        CALL    UNCOMPRESS
        RTS
    
### TODO:

- Allow multiple font ROMs
- Enable japanese input string when japanese font ROM is selected

 
