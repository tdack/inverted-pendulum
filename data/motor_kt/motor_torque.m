clear; clc;
%% Determine motor torque constant K_t
%
% Step voltage for each run was measured at 11.62V

k_t = [];
for j=1:6
    filename = strcat('results-', num2str(j));
    data = csvread(strcat(filename,'.csv'), 1,3);
    if j == 1
        total_t = data(1:555,1);
        total_v = data(1:555,2);
    else
        total_t = horzcat(total_t,data(1:555,1));
        total_v = horzcat(total_v,data(1:555,2));
    end
    figure(1)
        hold on;
            plot(data(1:555,1), data(1:555,2), 'DisplayName', [filename]);
            title('Motor Step Response');
            xlabel('Time (s)');
            ylabel('Velocity (degree/s)');
    figure(2)
        hold on;
            plot(data(40:555,1), data(40:555,2), 'DisplayName', [filename]);
    % Calculate k_t = Vin/velocity
    k_t = [k_t 11.62/mean(data(40:555,2))];
end
    figure (1)
        plot(mean(total_t,2),mean(total_v,2), 'k-', 'DisplayName', ['mean']);
        legend(gca, 'show');
    figure (2)
        legend(gca, 'show');

display('Mean torque constant:');
display(mean(k_t));