function autoreleasePoolPop()
%AUTORELEASEPOOLPOP Pops the current autorelease pool and releases all of
%its objects

prepareLibrary;

calllib('CortidQCT', 'CQCT_autoreleasePoolPop');

end

