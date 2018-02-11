#  CONTRIBUTING

![Fire](http://recc.robertelder.org/fire.gif "Fire") DO NOT CREATE PULL REQUESTS FOR THIS PROJECT.  ![Fire](http://recc.robertelder.org/fire.gif "Fire")

![Fire](http://recc.robertelder.org/fire.gif "Fire") ANY PULL REQUESTS YOU CREATE WILL NOT BE MERGED IN. ![Fire](http://recc.robertelder.org/fire.gif "Fire")

Contributing to this project is not currently permitted.  You are, however, encouraged to create forks.

Having said this, don't let the warning above prevent you from filing issues if you find something broken or undesirable.

#  THE FAST MEME TRANSFORM

This repository contains a tool that can be used to transform audio in .wav files into self-contained Linux commands that contain all the audio data, along with an audio codec.

For example, you can use the following commands to take audio extracted from a YouTube video and create a Linux command that plays the audio:

```
#  Clone this repo
git clone https://github.com/RobertElderSoftware/fast-meme-transform.git && cd fast-meme-transform
#  These instructions were tested to work correctly with this commit hash (optional step):
git checkout 05c48a3271697217f893a97d87eea29fb4ff6f90
#  Build Fast Meme Transform tool
make fmt
#  Get audio in PCM .wav format
youtube-dl M6PbdJiAK84 -x --audio-format wav -o "tripple.%(ext)s"
#  Build the self-contained Linux command
./fmt -f tripple.wav -s 4.9 -e 7.9
```

The resulting output will be a Unix command that plays the audio in the clip:

```
echo -n "uuuuJC8/CvOoO2/OgO5EGvHLHU:i:a:]<V<3/<4/Dz?3F?_19W19w19U19Z20P20p20[21VOVOdOWnCZ:V:2F:W:6>:_:8I:c<k<1><3/<y20Q20[20v21|21o2G/2H*2D(2?(21]OkOWOmO7E<5X<t<i<b<]D0>D1>D2(DqD4/D5YD6FD7>DaDc?l?{?W?5E9z9fDiDa?k?1=r?2=r?4Y?h?m?[n9z9Q?|?3I?4Br?5I?6>?7}190E9f?P?f?p?4/?5Br?6B5#?7/n8h8a8Z<m<8/D0>D2/DWDgD5>DtD8/Dc?|?o?2/?Ln7h7L7_7a8l8P8WCZ:o:d:p:g:s:b:]<0}<1><f<Q<5/<s<Un6i6v7V71>72(7Q75/7s7UCkCRCwCtC7E5_6f6p6R6L6Un51F5p54E4h4L4b49EJ77/78I8V8z83X8R39z39W39y397Fn80F8f<q40P4G%4C*4<%482F5N*56P56d56Wn8_DfSySmK|KoKp5HF5<*5?(TlTQN4^L87/DhKtKiKcTqTgTLTU6G(6H(78U78[n8b8Z4S%4K(jRjhjL532/533/62l62P62q799^b8Z4S(4K(jQj5/53P532/61v6O%798^a8]4S%4K(jgjsj7M02(80pn7Z80^P82M3M4M6>8i8cKQKmK[j0Xn8h86I8U8b8ZKgKyT{5O%NUN]61V869^y8t8_8[8cKQK_TzT3/T4/N[Nc61l6G^6M7M]9VKmTPTpTgNc61|6G(6H^w8tSRSsSaS]5:*5<*59bN|NoNqNRNyN6^y8m<]DV33vSP4OYS3YS4/S9^R85EuuJ7R7w7s77Y79>8k8W8Qn71X74>75=r76>77/78MP:k:{:3/:5E7U79}80=r8{82MW8R8L:Q:5/:L:Z<k<d<4^d83M4M5M6I87Y88MZ90>9{D3XDRDL26_26v27l272(27QnDiDa?l35R35w35t35[3N%j6Fn?z?f35[35c3N/36P363Xnu8V82Mg8s<f<Q<h<m<7^0M1Md:5X:m:U:8/:]<0=5#<{<d<W<Q<5/<sn7_7bCq:k:d:p:4=r:5>:6I:7Y:[<LnC2FCRChCsC_CaC9I:0Br:o:2>:3>:g:w:tn7fH7/ClCPC4B5#C5BrCLCiCaC]:knGWGwG7IG9>HkHoH2IHgH5}H6>Hin9L9b10q10R11V11{1G(1H%1D*1?*11cG|GPGLGUHlHUH[CoCqC5/CLC_C9F:l:q:h:L:Z<V<d<3/n1:*1?%GdGmGiGbGZHVH1E@n" | hexdump -v -e '/1 "%u\n"' | awk 'function d(f,k,a,b,o,w,i,q){for(k=32;k<127;k++)o[sprintf("%c", k)]=k;split("e-0,,, 2x,,=1#,=-1,,=2#,=-2,&#,+,,-,.,)#,0,1,2,3,4,5,6,7,8,9,15,;,16,=,=3#,18,0=0,9 3,=-,14,17,%n,BAx,12,13,B3#,@n,43,6*,/8,60,22,1*,4*,4%,42,52,7*,0%,3*,=Ax,B4#,9%,8*,,9(,E8,7%,,8(,8%,9/,2*,e,2%,4(,5*,7(,44,0(,0*,6%,\n,1/,3(,3%,6#,6/,6(,JJ,9*,5(,x,5%,1%,1(,0/,=4#,,",a,",");split("0,1,2,3,4,5,6,7,8,9,=,e,x,+,-,.,n,;",w,",");q=0;for(g in w)if(o[w[g]]==f)q++;i=f-32+1;if(q>0)printf(a[i]);else{split(a[i],b,"");for(j in b)d(o[b[j]]);}}d($1)' | awk 'BEGIN{L=2400;N=8192;for(n=0;n<L;n+=1)o[n+1]=0;}{l=split($1,t,"x");for(j=0;j<l;j+=1){split(t[j+1],p,"=");x[j+1]=p[2];f[j+1]=p[1];}M=(NR==60?2400:2*L);for(n=0;n<M;n+=1){R=0;for(j=0;j<l;j+=1){a=cos(2*3.141592*f[j+1]*(n/N)*1.000000);R+=(x[j+1]*a);}if(n<L){R=(R*(n/L))+(o[n+1]*((L-n)/L));R*=2.000000;R=int(R*32767);printf("%04X\n",R==0?0:(R>0?(R>32767?32767:R):(R<-32768?32768:65536+R)));}else if(n<2*L){o[n-L+1]=R;}}}' | tac | tac | xxd -r -p | aplay -q -c 1 -f S16_BE -r 48000
```

Depending on your host operating system, you may need to use the -k option to change which audio driver is used by default.  See details below.

You can read more about how this tool works at these links:

- [The Fast Meme Transform: Convert Audio Into Linux Commands](http://blog.robertelder.org/fast-meme-transform/)
- [Overlap Add, Overlap Save Visual Explanation](http://blog.robertelder.org/overlap-add-overlap-save/)
- [Endpoint Discontinuities and Spectral Leakage](http://blog.robertelder.org/endpoint-discontinuities-spectral-leakage/)
- [Fourier Transform Coefficients Of Real Valued Audio Signals](http://blog.robertelder.org/fourier-coefficients-audio-signals/)

#  BUILDING THE FAST MEME TRANSFORM TOOL

The fast meme transform tool uses a standard makefile setup to build itself.  The tool iteself is 100% written in C with no external dependencies other than the C standard library.  Simply run

```
make fmt
```

to build it.

#  COMMAND LINE OPTIONS

##  -k, --command-type

A single number that will determine which type of command will be generated to play the final audio.  You may need to use this option to generate a command that will play in your operating system without installing additional tools.  0 = use sox to automatically detect audio driver (Tested to work on Ubuntu 16), 1 = use aplay (tested on Ubuntu 16) 2 = sox with coreaudio (Probably works on a Mac, but I don't own one so I can't test this), 3 = sox with alsa.

###### Example
```
./fmt -f my_file.wav -k 1
```

##  -h, --help

Display this help menu.

###### Example
```
./fmt -f my_file.wav -h
```

##  -t, --threshold

A floating point number that describes the cutoff power level, below which any frequency with less than this quantity of power will not be included in the output.  If you make this number high, the output will be small, but the audio will sound terrible.  If you make this value low, the output will be huge and slow to decode, but the result will sound better. If you make this value large, the output will be small but the sound quality will decrease.

###### Example
```
./fmt -f my_file.wav -t 40
```



##  -s, --start-time

The start time in seconds where the output audio clip will begin.

###### Example
```
./fmt -f my_file.wav -s 10.3
```

##  -e, --end-time

The end time in seconds where the output audio clip will end.  An end time of zero indicates end of the clip.

###### Example
```
./fmt -f my_file.wav -e 10.3
```

##  -v, --verbose

Be verbose.

###### Example
```
./fmt -f my_file.wav -v
```

##  -f, --file

The file name of the input PCM .wav file to be processed.

###### Example
```
./fmt -f my_file.wav
```

##  -u, --disable-compression

Disable compression of the output Fourier coefficients.

###### Example
```
./fmt -f my_file.wav -u
```

##  -w, --enable-hann-window

Enable the application of a Hann window to each audio sample period.

###### Example
```
./fmt -f my_file.wav -w
```

##  -i, --include-imaginary

Include the imaginary parts of the coefficients of the Fourier transform output.

###### Example
```
./fmt -f my_file.wav -i
```

##  -a, --full-power-spectrum

Include the entire range of frequencies from the Fourier transform output instead of just including up to N/2.

###### Example
```
./fmt -f my_file.wav -a
```

##  -m, --volume

A floating point number describing the output volume.

###### Example
```
./fmt -f my_file.wav -m 3.2
```

##  -b, --enable-endpoint-discontinuities

Enable endpoint discontinuities that occur at the ends of sampling intervals (see -p flag). Enable this option if you want the presence of annoying high-frequency popping noises in your audio.

###### Example
```
./fmt -f my_file.wav -b
```

##  -p, --sample-time-period

A floating point number that describes how much long each sub-sample will be when the audio is broken up into small sections to have a Fourier transform applied to it.

###### Example
```
./fmt -f my_file.wav -p 0.03
```

##  -n, --float-precision

An integer describing how many digits should be printed after the decimal after for each Fourier coefficient that appears in the output.

###### Example
```
./fmt -f my_file.wav -n 5
```

##  -l, --filters

A list of length 3n that one or more sequence of a single character followed by two numbers that describe the parameters of either a low pass or high pass biquad filter.  Each biquad filter will be applied one after another on the waveform before the Fourier transform is calculated.  For example, using '-l l 5 500' will apply a lowpass filter with a Q value of 5, and a cutoff frequency of 500.  The extra character 'l' or 'h' is for low pass and high pass respectively.

###### Example
```
./fmt -f my_file.wav -l l 5 500
./fmt -f my_file.wav -l h 3 600
```

##  -c, --pitch

A positive number that can be used to either shift the frequency up or down.  Set this value greater than one to increase the pitch, and set it between 0 and 1 to decrease the pitch.

###### Example
```
./fmt -f my_file.wav -l l 5 500
./fmt -f my_file.wav -l h 3 600
```

#  LICENSE

See LICENSE.txt
