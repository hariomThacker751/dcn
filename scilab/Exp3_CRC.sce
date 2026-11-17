// Experiment 3: CRC (Cyclic Redundancy Check) generation and error detection
// Sender:   append (n-1) zeros to the data, divide by the generator using
//           modulo-2 division; the remainder is the CRC. Codeword = data + CRC.
// Receiver: divide the received word by the same generator. Remainder zero
//           means no error, non-zero remainder means an error is detected.

clear;

function rem = mod2div(dividend, divisor)
    // modulo-2 (XOR) long division, returns the remainder
    n = length(divisor);
    temp = dividend;
    for i = 1:(length(dividend) - n + 1)
        if temp(i) == 1 then
            temp(i:i+n-1) = bitxor(temp(i:i+n-1), divisor);
        end
    end
    rem = temp($-n+2:$);
endfunction

data = [1 0 1 1 0 1 1];   // data bits
gen  = [1 0 1 1];         // generator polynomial x^3 + x + 1

// ---------- Sender ----------
padded = [data zeros(1, length(gen) - 1)];
crc = mod2div(padded, gen);
codeword = [data crc];

disp("Data      : ", data);
disp("Generator : ", gen);
disp("CRC       : ", crc);
disp("Codeword  : ", codeword);

// ---------- Receiver: no error ----------
r1 = codeword;
s1 = mod2div(r1, gen);
disp("Received (no error) : ", r1);
disp("Remainder           : ", s1);
if and(s1 == 0) then
    disp("No error detected");
else
    disp("Error detected");
end

// ---------- Receiver: with error ----------
r2 = codeword;
r2(4) = 1 - r2(4);   // error introduced in bit 4
s2 = mod2div(r2, gen);
disp("Received (bit 4 flipped) : ", r2);
disp("Remainder                : ", s2);
if and(s2 == 0) then
    disp("No error detected");
else
    disp("Error detected");
end
