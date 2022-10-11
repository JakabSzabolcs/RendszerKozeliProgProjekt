# RendszerKozeliProgProjekt
4. féléves tárgyam projektmunkája.

Rövid leírása a programról:

A program futtatáskor kettő darab kapcsolót vár. Ezekkel a kapcsolókkal beállíthatjuk a program módját(fogadó vagy küldő) valamint a típusát(socket vagy file). Alapértelmezetten küldő üzemmódban és file típusban van. A program küldő módban az utolsó negyedóra óta eltelt másodpercek nagyságában generál egy számsorozatot, ahol két szomszédos szám maximum különbsége 1 lehet abszolútértékben, és 0 a kezdőérték hiszen egy képzeletbeli szenzorról beszélünk. Ezt a számsorozatot a küldő továbbítja a fogadó programnak. Ezt küldheti file-on keresztül, .txt kiterjesztésű file-ként, vagy egy socket aljzaton. A .txt file-t a küldő a /home mappába menti el. A fogadó módban futó program ezt az adatsorozatot egy BMP kiterjesztésü file-ra alakítja. Ez a BMP file, 1bit színmélységű kép, azaz minden pixel 1 biten van eltárolva. Ezt a BMP file-t a projekt mappában hozza létre chart.bmp néven, ahol a fogadó program el lett indítva.
