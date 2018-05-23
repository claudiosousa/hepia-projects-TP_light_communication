% Reconstitustion d'un message reçu par signal lumineux à 2 composantes (rouge et bleue)
% Le fichier contenant le signal d'entrée doit alterner les échantillons clear/RGB
% sous forme "texte" avec un nombre par ligne dans le fichier de votre
% choix (nom à donner dans 'filename').
clear
filename='rgb_ref.txt';

samples_per_bit=5;
tot_bytes=15;    % nombre d'octets reçus incluant les 2 octets de la séquence de référence
bits_ref=[1 0 0 1 0 1 0 1 0 0 0 1 1 0 1 1];    % séquence de référence(0x951B)

% construction de la séquence en tenant compte de samples_per_bit
seq_ref_red=[];
seq_ref_blue=[];
for n=0:2:length(bits_ref)-1
    seq_ref_red=[seq_ref_red (2*bits_ref(n+1)-1)*ones(1,samples_per_bit)];
    seq_ref_blue=[seq_ref_blue (2*bits_ref(n+2)-1)*ones(1,samples_per_bit)];
 end
r=load(filename);
rlen=length(r);
red(1:rlen/4)=r(2:4:end);
blue(1:rlen/4)=r(4:4:end);

max_sig=max(blue);
max_ratio=max_sig/max(red);
red=red*max_ratio;            % normalisation amplitude  


figure(1)
plot(1:rlen/4, red, 'r', 1:rlen/4, blue, 'b')
title('Signaux rouge et bleu reçus normalisés par rapport au signal bleu')

figure(2)
clf
subplot(5,1,1)
plot(red, 'r')
axis([0 length(red) -50 1.2*max_sig])
title('Signal rouge reçu (normalisé)')

subplot(5,1,2)
plot(blue)
axis([0 length(blue) -50 1.2*max_sig])
title('Signal bleu reçu')

subplot(5,1,3)
c_red=conv(red,seq_ref_red(end:-1:1));
corr_res_red=c_red(length(seq_ref_red):end);
plot(corr_res_red,'r--')
title('Corrélation du signal rouge avec séquence de référence')

subplot(5,1,4)
c_blue=conv(blue,seq_ref_blue(end:-1:1));
corr_res_blue=c_blue(length(seq_ref_blue):end);
plot(corr_res_blue,'b--')
title('Corrélation du signal bleu avec séquence de référence')

subplot(5,1,5)
corr_tot=corr_res_red+corr_res_blue;
plot(corr_tot,'k')
% recherche de l'indice du maximum de corrélation sur le premier tiers
% du signal
[m idx]=max(corr_tot(1:ceil(end/3)));
sprintf('Position du début de trame: %d', idx)
start=idx(1)+floor(samples_per_bit/2)
title(sprintf('Corrélation de la somme (rouge+bleu). Première position de S: %d', start))


% index des bits de données: 
t=start:samples_per_bit:start+(tot_bytes*8/2-1)*samples_per_bit;  % /2 car 2 composantes
bits(1:2:tot_bytes*8)=red(t)>max_sig/2;    % le signe de f(t) indique la valeur de chaque bit
bits(2:2:tot_bytes*8)=blue(t)>max_sig/2;

% Transformation binaire -> hexadécimal -> ASCII
z=binaryVectorToHex(bits');
znum=[];
for n=5:2:length(z)
    znum=[znum hex2dec(z(n:n+1))];
end
% détection de la séquence de référence
if strcmp(z(1:4),'951B')
    sprintf('Reception correcte: %s (%s)\n', [char(znum)], z)
else
    sprintf('Mauvaise reception (%s (%s))\n', [char(znum)], z)
end

figure(3)
clf
subplot(2,1,1)
range=start:start+(tot_bytes*8-1)*samples_per_bit;  
plot(range-start+1,red(range),'r')    
title('Illustration de la détection des bits sur signal rouge alignés sur S')
hold on
plot(t-start+1, red(t), 'kx', t(1:8)-start+1, bits_ref(1:2:end)*max_sig, 'mo')
for n=1:samples_per_bit*8:length(range);
    line([n n],[-100 max_sig*1.2],'Color',[.5 .5 .5])
end

subplot(2,1,2)
range=start:start+(tot_bytes*8-1)*samples_per_bit; 
plot(range-start+1,blue(range),'b')    
title('Illustration de la détection des bits sur signal bleu alignés sur S')
hold on
plot(t-start+1, blue(t), 'kx', t(1:8)-start+1, bits_ref(2:2:end)*max_sig, 'mo')
for n=1:samples_per_bit*8:length(range);
    line([n n],[-100 max_sig*1.2],'Color',[.5 .5 .5])
end
