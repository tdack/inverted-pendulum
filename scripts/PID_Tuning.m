% Calculate PID parameters based on angle measurement from
% pendulum position encoder

clear; clc;

% read data
data = csvread("kp_10.csv");
Ku = 10;
figure
  plot(cumsum(data(:,1)), data(:,2)*180/pi);
  
T = mean(data(:,1)); % sample delta T
Fs = 1/T;            % frequency of samples
L=length(data(:,1)); % number of samples

% fourier transform of angle
Y = fft(data(:,2));

% Single sided frequency spectrum
P2=abs(Y/L);
P1=P2(1:L/2+1); P1(2:end-1)=2*P1(2:end-1);
f = Fs*(0:(L/2))/L;
figure
  plot(f,P1);

% Find largest spike in fourier transform
[m, ind] = max(P1);

Tu = 1/f(ind);

printf("Tu=%.4f  Ku=%.4f\n\n", Tu, Ku);

% Pessen Integral Rule
Kp = 0.7*Ku; Ki=2.5*Kp/Tu; Kd=3*Kp*Tu/20;
printf("Pessen\nKp=%.4f Ki=%.4f Kd=%.4f\n\n", Kp, Ki, Kd);

% Some overshoot
Kp = 0.33*Ku; Ki=2*Kp/Tu; Kd=Kp*Tu/3;
printf("Overshoot\nKp=%.4f Ki=%.4f Kd=%.4f\n\n", Kp, Ki, Kd);

% Classic PID
Kp = 0.6*Ku; Ki=2/Tu; Kd=Tu/8;
printf("Classic\nKp=%.4f Ki=%.4f Kd=%.4f\n\n", Kp, Ki, Kd);

% Z-N PI
Kp = 0.45*Ku; Ki=1.2/Tu; Kd=0;
printf("Z-N PI\nKp=%.4f Ki=%.4f Kd=%.4f\n\n", Kp, Ki, Kd);
