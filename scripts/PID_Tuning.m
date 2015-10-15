% Calculate PID parameters based on angle measurement from
% pendulum position encoder

data = kp70;
Ku = 70;

figure
    plot(cumsum(data(:,1)), data(:,2)*180/pi);
    title('Pendulum Angle vs Time');
    xlabel('Time (s)');
    ylabel('Pendulum Angle (degrees)');

T = mean(data(:,1)); % sample delta T
Fs = 1/T;            % frequency of samples
L=length(data(:,1)); % number of samples

figure
    meanfreq(data(:,2), Fs);

Tu = 1/meanfreq(data(:,2), Fs);

fprintf('Tu=%.4f  Ku=%.4f\n\n', Tu, Ku)

% Pessen Integral Rule
Kp = 0.7*Ku; Ki=Ku*Tu/2.5; Kd=Ku*3*Tu/20;
fprintf('Pessen\nKp=%.4f Ki=%.4f Kd=%.4f\n\n', Kp, Ki, Kd)

% Some overshoot
Kp = Ku/3; Ki=Ku*Tu/2; Kd=Kp*Tu/3;
fprintf('Overshoot\nKp=%.4f Ki=%.4f Kd=%.4f\n\n', Kp, Ki, Kd)

% Classic PID
Kp = 0.6*Ku; Ki=Ku*Tu/2; Kd=Ku*Tu/8;
fprintf('Classic\nKp=%.4f Ki=%.4f Kd=%.4f\n\n', Kp, Ki, Kd)

% Z-N PI
Kp = 0.45*Ku; Ki=Ku*Tu/1.2; Kd=0;
fprintf('Z-N PI\nKp=%.4f Ki=%.4f Kd=%.4f\n\n', Kp, Ki, Kd)
