*** Settings ***
Suite Setup                   Setup
Suite Teardown                Teardown
Test Setup                    My Test Setup
Test Teardown                 Test Teardown
Resource                      ${RENODEKEYWORDS}

*** Keywords ***
My Test Setup
    Reset Emulation
    Execute Command         path add @${CURDIR}

Feed Constant
    [Arguments]       ${sample}  ${channel}
    Execute Command   sysbus.adc1 FeedSample ${sample} ${channel} -1

# Feed SawTooth
#     [Arguments]       ${sample}
#     Execute Command   sysbus.adc1 FeedSample ${sample}
#
# Feed SineWave
#     [Arguments]       ${sample}
#     Execute Command   sysbus.adc1 InputConstant ${sample}

Received Constant
    [Arguments]             ${sample}
    Wait For Line On Uart   ADC Data: ${sample}  timeout=0.2
    Wait For Line On Uart   ADC Data: ${sample}  timeout=0.2
    Wait For Line On Uart   ADC Data: ${sample}  timeout=0.2

Received ADC Data On Channel
    [Arguments]             ${channel}  ${sample}
    Wait For Line On Uart   Channel: ${channel} ADC Data: ${sample}  timeout=0.2

# Received SawTooth
#     [Arguments]             ${start}
#     Wait For Line On Uart   ADC Data: ${start}  timeout=0.2
#     Wait For Line On Uart   ADC Data: ${start} + 1  timeout=0.2
#     Wait For Line On Uart   ADC Data: ${start} + 2  timeout=0.2

*** Test Cases ***
Polling ADC can read sample
    Execute Command         include @polling.resc

    Create Terminal Tester  sysbus.uart1

    Start Emulation

    Feed Constant  sample=1  channel=1
    Received Constant  sample=1

    Feed Constant  sample=5  channel=1
    Received Constant  sample=5

Polling can scan ADC channels
    Execute Command         include @polling_scan.resc

    Create Terminal Tester  sysbus.uart1

    Feed Constant  sample=1  channel=1
    Feed Constant  sample=2  channel=2
    Feed Constant  sample=3  channel=3

    Start Emulation

    Received ADC Data On Channel  channel=1  sample=1
    Received ADC Data On Channel  channel=1  sample=1
    Received ADC Data On Channel  channel=1  sample=1

Scan mode can covert regular channel group
    Execute Command         include @continuous_scan.resc

    Create Terminal Tester  sysbus.uart1

    Feed Constant  sample=1  channel=1
    Feed Constant  sample=2  channel=2
    Feed Constant  sample=3  channel=3

    Start Emulation

    Wait For Line On Uart   Received All Channels: 1 2 3  timeout=0.2

DMA ADC can read sample
    Execute Command         include @dma.resc

    Create Terminal Tester  sysbus.uart1

    Start Emulation

    Feed Constant  sample=1  channel=1
    Received Constant  sample=1

    Feed Constant  sample=5  channel=1
    Received Constant  sample=5
