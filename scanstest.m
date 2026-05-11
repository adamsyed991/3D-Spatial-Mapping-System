%% Deliverable 2 Code
%syeda139
%Adam Syed
%400578109

port = "COM3";
baudrate = 115200;

% Open serial port
device = serialport(port, baudrate);
device.Timeout = 30;   % slightly longer in case motion takes time
configureTerminator(device, "CR/LF");
flush(device);

disp("Serial port opened.");
disp("Waiting to automatically collect 3 scans...");

% X positions for the 3 scans
x_positions = [0 100 200];

% Storage for all scans
X_all = [];
Y_all = [];
Z_all = [];

num_scans = 3;

for scan_num = 1:num_scans
    fprintf("Waiting for Scan %d of %d\n", scan_num, num_scans);

    angle_deg = [];
    distance_mm = [];
    range_status = [];

    % Wait for SCAN_START
    disp("Waiting for SCAN_START...");
    while true
        line = char(readline(device));
        line = strtrim(line);

        if strcmp(line, 'SCAN_START')
            fprintf("SCAN_START received for scan %d.\n", scan_num);
            break;
        end
    end

    % Read until SCAN_END
    disp("Reading scan data...");
    while true
        line = char(readline(device));
        line = strtrim(line);

        if strcmp(line, 'SCAN_END')
            fprintf("SCAN_END received for scan %d.\n", scan_num);
            break;
        end

        % Skip header line
        if contains(line, 'index')
            continue;
        end

        % Split CSV line
        parts = strsplit(line, ',');

        if numel(parts) ~= 4
            continue;
        end

        % Extract fields
        angle_deg(end+1,1) = str2double(parts{2});
        range_status(end+1,1) = str2double(parts{3});
        distance_mm(end+1,1) = str2double(parts{4});
    end

    % Keep only valid points
    valid = (range_status == 0) & (distance_mm > 0);
    angle_deg = angle_deg(valid);
    distance_mm = distance_mm(valid);

    % Convert from polar to Cartesian
    theta = deg2rad(angle_deg);

    x_current = x_positions(scan_num) * ones(size(theta));
    y_current = distance_mm .* cos(theta);
    z_current = distance_mm .* sin(theta);

    % Save this scan into full dataset
    X_all = [X_all; x_current];
    Y_all = [Y_all; y_current];
    Z_all = [Z_all; z_current];

    fprintf("Stored %d valid points for scan %d at x = %d mm.\n", ...
        length(x_current), scan_num, x_positions(scan_num));
end

% Close serial port
clear device;
disp("Serial port closed.");

% Plot all 3 scans as separate connected closed curves
% y and z are swapped here
figure;
hold on;

for scan_num = 1:num_scans
    idx = (X_all == x_positions(scan_num));

    x_scan = X_all(idx);
    y_scan = Y_all(idx);
    z_scan = Z_all(idx);

    % Add first point again at the end to close the curve
    x_plot = [x_scan; x_scan(1)];
    y_plot = [z_scan; z_scan(1)];   % swapped
    z_plot = [y_scan; y_scan(1)];   % swapped

    plot3(x_plot, y_plot, z_plot, '-o', ...
        'LineWidth', 1.5, 'MarkerSize', 5);
end

xlabel('x (mm)');
ylabel('z (mm)');
zlabel('y (mm)');
grid on;
axis equal;
view(3);

% Force identical numeric limits on all axes
all_vals = [X_all; Y_all; Z_all];
min_val = min(all_vals);
max_val = max(all_vals);

xlim([min_val max_val]);
ylim([min_val max_val]);
zlim([min_val max_val]);

hold off;