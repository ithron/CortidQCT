function saveModel(model,filename)
%SAVEMODEL Saves the given model to the given file in YAML format

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

    function encodedData = encodeData(data)
        encodedData = CortidQCT.floatVector2base64(single(data(:)'));

        % prettify encoded data string
        prettyfiedData = '';
        cnt=0;
        for ii=1:length(encodedData)
            prettyfiedData(end+1) = encodedData(ii);
            cnt = cnt + 1;
            if cnt == 72
                prettyfiedData(end+1:end+7) = sprintf('\n      ');
                cnt = 0;
            end
        end
        
        encodedData = prettyfiedData;
    end

fileId = fopen(filename, 'w');

fprintf(fileId, '---\n');

fprintf(fileId, 'version: %s\n', model.version);

if isfield(model, 'name')
    fprintf(fileId, 'name: %s\n', model.name);
end

if isfield(model, 'description')
    fprintf(fileId, 'description: |-\n  %s\n', strjoin(model.description, '\n  '));
end

if isfield(model, 'author')
    fprintf(fileId, 'author: %s\n', model.author);
end

if isfield(model, 'creationDate')
    fprintf(fileId, 'creationDate: %s\n', model.creationDate);
end

fprintf(fileId, 'kernel:\n');
fprintf(fileId, '  sigma: !!float %g\n', model.kernel.sigma);

fprintf(fileId, 'sliceSpacing: !!float %g\n', model.sliceSpacing);

fprintf(fileId, 'samplingRange:\n');
fprintf(fileId, '  min: !!float %g\n', model.samplingRange.min);
fprintf(fileId, '  max: !!float %g\n', model.samplingRange.max);
fprintf(fileId, '  stride: !!float %g\n', model.samplingRange.stride);

fprintf(fileId, 'densityRange:\n');
fprintf(fileId, '  min: !!float %g\n', model.densityRange.min);
fprintf(fileId, '  max: !!float %g\n', model.densityRange.max);
fprintf(fileId, '  stride: !!float %g\n', model.densityRange.stride);

fprintf(fileId, 'angleRange:\n');
fprintf(fileId, '  min: !!float %g\n', model.angleRange.min);
fprintf(fileId, '  max: !!float %g\n', model.angleRange.max);
fprintf(fileId, '  stride: !!float %g\n', model.angleRange.stride);

fprintf(fileId, 'density:\n');

for voiEl=model.VOIs
    voi = voiEl{1};
    fprintf(fileId, '  - name: %s\n', voi.name);
    fprintf(fileId, '    label: %d\n', voi.label);
    fprintf(fileId, '    mean: [%d, %d, %d]\n', ...
            voi.mean(1), voi.mean(2), voi.mean(2));
    fprintf(fileId, '    std: [%d, %d, %d]\n', voi.std(1), voi.std(2), voi.std(2));
    fprintf(fileId, '    muW: %d\n', voi.muW);
    fprintf(fileId, '    sigmaW: %d\n', voi.sigmaW);
    fprintf(fileId, '    scale: %d\n', voi.scale);
    encDat = encodeData(voi.data);
    fprintf(fileId, '    data: !binary |-\n      %s\n', encDat);
end

fclose(fileId);

end

