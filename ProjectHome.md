This package contains algorithms for quickly finding all maximal sets with respect to set containment among an input collection of sets.

  * Click on the "Source" tab for instructions on downloading the source code. Makefiles are provided for GNU g++ and Microsoft VC++ compilers.

  * Click on the "Downloads" tab to download a sample dataset to test your binary.

You can find a research paper describing these algorithms at: http://www.bayardo.org/ps/sdm2011.pdf


---


On Linux type systems with GNU g++, here's a quick sequence of commands to get up and running:

```
[~/ams-demo]: svn checkout http://google-extremal-sets.googlecode.com/svn/trunk/ google-extremal-sets-read-only
...
Checked out revision 2.
[~/ams-demo]: cd google-extremal-sets-read-only/
[~/ams-demo/google-extremal-sets-read-only]: make
...
[~/ams-demo/google-extremal-sets-read-only]: wget http://google-extremal-sets.googlecode.com/files/dblp_lex_sorted.bin.gz
...
10:20:59 (4.91 MB/s) - 'dblp_lex_sorted.bin.gz' saved [26061298/26061298]
[~/ams-demo/google-extremal-sets-read-only]: gunzip dblp_le_fixed.bin.gz
[~/ams-demo/google-extremal-sets-read-only]: ./ams-lexicographic dblp_lex_sorted.bin
; Finding all maximal itemsets.
; Limit on number of items in main memory: 1000000000
; Starting new dataset scan at offset: 0
; Potential maximal sets: 781350
; Beginning subsumption checking scan.
; Dumping maximal sets.
; Found 773877 maximal itemsets.
; Number of itemsets in the input: 781514
; Number of candidate seeks performed: 21595380
; Total running time: 2 seconds
```


---


On Windows systems with Microsoft Visual C++, at the command-line run "vcvars32.bat" from your Visual C++ install location (e.g. C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32), then you can compile with:

```
 c:\google-extremal-sets>nmake -f Makefile.w32
```

