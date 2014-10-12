%% Setup

close all
clear all
clc

command_rate = 500;                 % [Hz] - Rate of command update rate
measurement_variance = 0.000000001; % [~]  - 1sigma variance of measurement noise
measurement_rate = 10;              % [Hz] - Rate of measurement update rate
measurement_delay = 1;              % [us] - Number of 1 microsecond delays in measurement update
bin_size = 10;                      % [~] - Number of bins for predictive filter
filter_order = 3;                   % [~] - Polynomial fit order of predictive filter
position_time_constant = 1000;      % [~] - Time constant for object position transfer function for command to true

%% Run the simulation and plot results

sim('TrackingTest')

% Plot position
figure, hold on
plot(x_target_true,'-b','linewidth',2)
plot(x_target_measured,'-r','linewidth',2)
plot(x_object_true,'-g','linewidth',2)
plot(x_object_commanded,'--k')
xlabel('Time [s]')
grid on
ylabel('Position [~]')
legend('True Target Position','Measured Target Position','True Object Position','Commanded Object Position')

% Plot position error
figure, hold on
plot(x_target_true - x_object_true,'linewidth',2);
xlabel('Time [s]')
ylabel('Position Error [~]')