function autoreleasePoolPush()
%AUTORELEASEPOOLPUSH Pushes a new autorelease pool

prepareLibrary;

calllib('CortidQCT', 'CQCT_autoreleasePoolPush');

end

