# Navtex
Sdr Navtex receiver + react javascript based gui


The main dish of this project is an sdr (software defined radio) experiment. 
As I am a recreational sailor owning a GMDSS GOC licence, a licenced Ham operator, and an engineer with some knowledge of electronics, telecommunications and computers, I though it was a good project to go for. 
The main trigger is the fact that no good and usable navtex decoding software for use in leisure crafts exists at this moment.
Projects like openplotter have brought together a great deal of excellent software to cover a lot of the navigation and comminication needs of a recreational boat. 
However a navtext receiver is still a missing component in that ecosystem. 
Sure there's software like YAND and FLDigi. The thing is that those are just meant for the occasional HAM that wants to capture some navtex messages for fun. 
As a boater you need to be able to receive those messages without being in front of a GUI at the exact moment the messages are transmitted. Reception should happen in background. Next to that Navtex software should allow you to select message types (subject indicator) and message sources (transmitter identity). So the B1 and B2 characters of the navtex message. 

So I decided to create this background process that allows navtex demodulation and decoding. 
Next to that I created a web-based GUI application for displaying messages. 

