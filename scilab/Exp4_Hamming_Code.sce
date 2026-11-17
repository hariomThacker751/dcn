// Experiment 4: (7,4) Hamming code for error detection and correction
// Sender:   c = d * G (mod 2), G = [I P]
// Receiver: s = H * r' (mod 2), H = [P' I]; a non-zero syndrome matches
//           the column of H at the position of the corrupted bit.

clear all;
d = [1 0 1 1]; //data bits
P = [1 1 0; 0 1 1; 1 1 1; 1 0 1];
G = [eye(4,4) P]; //generator matrix
H = [P' eye(3,3)]; //parity check matrix

c = modulo(d * G, 2); //codeword

r = c;
r(3) = 1 - r(3); //error introduced in bit 3

s = modulo(H * r', 2); //syndrome

pos = 0;
for i = 1:7
    if and(s == H(:,i)) then
        pos = i;
    end
end

disp("Data     : ", d);
disp("Codeword : ", c);
disp("Received : ", r);
disp("Syndrome : ", s');

if pos == 0 then
    disp("No error in received data");
else
    disp("Error at bit " + string(pos));
    r(pos) = 1 - r(pos);
    disp("Corrected: ", r);
    disp("Recovered: ", r(1:4));
end
