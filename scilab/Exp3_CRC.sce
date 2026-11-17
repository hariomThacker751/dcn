clc;
clear;

// 1. Inputs: Message and Generator Polynomial
data = [1, 1, 0, 1];        // Original message
poly = [1, 0, 1, 1];        // Generator polynomial (CRC-3)

n_data = length(data);
n_poly = length(poly);
k = n_poly - 1;             // Number of CRC bits to append

// 2. Padding Zeros to Message
padded_data = [data, zeros(1, k)];

// 3. Modulo-2 Division (CRC Generation)
temp = padded_data;
for i = 1:n_data
    if temp(i) == 1 then
        for j = 1:n_poly
            temp(i + j - 1) = bitxor(temp(i + j - 1), poly(j));
        end
    end
end

// Extract remainder (last k bits)
crc = temp(n_data + 1 : $);
tx_code = [data, crc];       // Final codeword to send

disp("Original Data: ", data);
disp("CRC Checksum:  ", crc);
disp("Transmitted:   ", tx_code);

// 4. Receiver Error Check
rx_code = tx_code;           // Change any bit to simulate an error (e.g. rx_code(2) = 0)
temp_rx = rx_code;

for i = 1:n_data
    if temp_rx(i) == 1 then
        for j = 1:n_poly
            temp_rx(i + j - 1) = bitxor(temp_rx(i + j - 1), poly(j));
        end
    end
end

remainder = temp_rx(n_data + 1 : $);

if sum(remainder) == 0 then
    disp("Status: SUCCESS - No errors detected!");
else
    disp("Status: ERROR - Data corrupted during transmission!");
end
