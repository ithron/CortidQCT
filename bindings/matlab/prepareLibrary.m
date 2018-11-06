function prepareLibrary()
%PREPARELIBRARY Loads the CortidQCT library if it's not loaded

if not(libisloaded('CortidQCT'))
    hfile = sprintf('%s%s', getenv('CortidQCT_ROOT'), '/include/CortidQCT/Matlab/CortidQCT.h');
    hfile2 = sprintf('%s%s', getenv('CortidQCT_ROOT'), '/include/CortidQCT/Matlab/cortidqct-Matlab_exports.h');
    libfile = sprintf('%s%s',getenv('CortidQCT_ROOT'), '/lib/libCortidQCT-Matlab.so');

    loadlibrary(libfile, hfile, 'addheader', hfile2, 'alias', 'CortidQCT');
end
