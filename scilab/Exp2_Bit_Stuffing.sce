// Experiment 2: Bit stuffing and de-stuffing (HDLC style)
// A 0 is inserted after every five consecutive 1s, then the data is
// framed with the 01111110 flag. The receiver strips the flags and
// removes the stuffed 0s to recover the original data.

x = "1111011111100111";

// ---------- Bit stuffing ----------
count = 0;
result = "";

for i = 1:length(x)
    result = result + part(x, i);
    if part(x, i) == "1" then
        count = count + 1;
    else
        count = 0;
    end
    if count == 5 then
        result = result + "0";
        count = 0;
    end
end

delimiter = "01111110";
frame = delimiter + result + delimiter;

// ---------- De-stuffing ----------
payload = part(frame, 9:length(frame)-8);

count = 0;
destuffed = "";
skip = %f;

for i = 1:length(payload)
    bit = part(payload, i);
    if skip then
        // this is the 0 inserted after five 1s, drop it
        skip = %f;
        count = 0;
        continue;
    end
    destuffed = destuffed + bit;
    if bit == "1" then
        count = count + 1;
    else
        count = 0;
    end
    if count == 5 then
        skip = %t;
        count = 0;
    end
end

disp("Original Data    : " + x);
disp("Bit Stuffed Data : " + result);
disp("Prepared Frame   : " + frame);
disp("Received Payload : " + payload);
disp("Destuffed        : " + destuffed);

if destuffed == x then
    disp("De-stuffed data matches the original data");
else
    disp("De-stuffed data does NOT match the original data");
end
