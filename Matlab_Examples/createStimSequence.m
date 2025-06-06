clear;
close all;
clc;

%% set name for experiment logfile
logFilename = 'Pilot01Seq';

%% define number of trials in this block
numTrials = 150; % should be an integer multiple of 5 to address each LED equally often

%% define basic LED array states
% base states, all LEDs at same brightness level
LED0 = {'*stim,a,a,a,a,a,a,'};
% 5 types of trials, one LED set to target brightness
% brightness is defined by variables a & b stored on the device by the
% staircase routine (only works if variables in device are not cleared)
LED{1} = '*stim,a,b,a,a,a,a,';
LED{2} = '*stim,a,a,b,a,a,a,';
LED{3} = '*stim,a,a,a,b,a,a,';
LED{4} = '*stim,a,a,a,a,b,a,';
LED{5} = '*stim,a,a,a,a,a,b,';

%% create random sequence of target types
trialSeq=repmat(1:5,1,numTrials/5)'; %initialize as fixed sequence (1:5)
trialSeq=trialSeq(randperm(length(trialSeq))); %permute sequence

%% define duration of target stimulus (milliseconds)
target_dur = 10;

%% define vector of ISIs (simple version)
% ISI = rand(1,numTrials)+1.5; % ISI between 1.5 and 2.5 sec
% ISI = round(ISI*1000);


%% Randomly Generate The ISI Duration
% ISI will have a lognormal distribution, range between ~1500ms and ~7000ms
% mean ~2800ms, SD ~700ms
R = zeros(numTrials,1);
sigma = sqrt( log( 700^2 / (2800^2) + 1) );
mu = log( 2800/exp(0.5*sigma^2) );

% Generate 150 ISI, ensuring that the minimum is greater than 1500ms
while min(R) < 1500
    R = lognrnd(mu,sigma,numTrials,1);
end
ISI = round(R);

%% open text file to store experiment
fId=fopen('stim_seq.txt','w');

%% define header 
% old variables are not cleared so the script can use brightness levels
% from the staircase routine (stored in vars a and b)
fwrite(fId,'clear_stim') %clear all old commands still in memory
fwrite(fId, newline);
fwrite(fId, ['log,open,' logFilename]) %open logfile
fwrite(fId, newline);

%% define random trial sequence
for trl = 1:numTrials
    % write trial number as comment
    fwrite(fId, ['%Trial ' num2str(trl)]);
    fwrite(fId, newline);


    % write baseline LED state with a random ISI
    fwrite(fId,[LED0{1}, num2str(ISI(trl))]);
    fwrite(fId, newline);


    % write target LED state with target duration
    fwrite(fId,[LED{trialSeq(trl)} num2str(target_dur)]);
    fwrite(fId, newline);
end

%% start execution of commands on device
fwrite(fId,'start_stim')
fwrite(fId, newline);

%% close log file
fwrite(fId,'log,close')
fwrite(fId, newline);

%% finalize textfile creation
fclose(fId);
