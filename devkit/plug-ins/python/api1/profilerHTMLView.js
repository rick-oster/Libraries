function toColor(value) 
{
	return "#" + (Math.round(value * 0XFFFFFF)).toString(16);
}

window.onload = function() 
{
		var fileInput = document.getElementById('fileInput');
		var eventList = document.getElementById('elementList');
		var eventCount = document.getElementById("eventCount");
		var canvas = document.getElementById("canvas");
		
		fileInput.addEventListener('change', function(e) {
			var file = fileInput.files[0];
			var textType = /text.*/;

			var reader = new FileReader();

			reader.onload = function(e)
			{
				var jsonText = reader.result;
				var jsonObject = JSON.parse(jsonText);

				// Write header
				eventHeader.innerHTML = 'Profiler Version: ' + jsonObject.version + "<br>";
			 	eventHeader.innerHTML += 'Total Event Count: ' + jsonObject.eventCount + "<br>";
				eventHeader.innerHTML += 'Events Written: ' + jsonObject.eventsWritten + "<br>";
				eventHeader.innerHTML += 'CPU Count: ' + jsonObject.cpuCount + "<br>";
				if (jsonObject.categories)
				{
					eventHeader.innerHTML += 'Categories:';
					eventHeader.innerHTML += '<ol>';
					for (i=0; i<jsonObject.categories.length; i++)
					{
						eventHeader.innerHTML += '<li>[' + i + '] = ' + jsonObject.categories[i];
					}
					eventHeader.innerHTML += '</ol>';
				}
				if (jsonObject.eventNames)
				{
					eventHeader.innerHTML += '<br>Event Names:';
					eventHeader.innerHTML += '<ol>';
					for (i=0; i<jsonObject.eventNames.length; i++)
					{
						eventHeader.innerHTML += '<li>[' + i + '] = ' + jsonObject.eventNames[i];
					}
					eventHeader.innerHTML += '</ol>';
				}

				var i;
				var eventTableInput = jsonObject.events;
      			var eventTable = "<table border=1>";

				// Table header items are the key labels
				eventTable += "<tr>";

				var haveDuration = false;
				var haveColorId = false;
				var categoriesArray = [];
				var categoriesColors = [];
				var categoryKey = "";
				if (eventTableInput.length)
				{
					eventRow =  eventTableInput[0];
					// Example to sort by duration descending 
					if ("duration" in eventRow)
					{
						haveDuration = true;
						eventTableInput.sort(
							function(a,b)
							{
								return b.duration-a.duration;
							});
					}
					if ("colorId" in eventRow)
					{
						haveColorId = true;
					}
					
					for (var key in eventRow)
					{
						eventTable += "<td><b>" + key.toUpperCase() + "</b></td>";
					}
			
					var categories = new Set();
					for(i = 0; i < eventTableInput.length; i++)		
					{
						eventRow =  eventTableInput[i];
						if ("catIdx" in eventRow)
						{
							categories.add(eventRow['catIdx'])
							categoryKey = "catIdx";
						}
						else if ("category" in eventRow)
						{
							categories.add(eventRow['category'])
							categoryKey = "category";
						}
					}
					categories.forEach(v => categoriesArray.push(v));
					categories.forEach(v => categoriesColors.push(Math.random()));
					console.log("Categories: " + categoriesArray);
					console.log("Categories: " + categoriesColors);					
				}
				eventTable += "</tr>";

				// Row items are key data
				var durationData = [];	
				var durationTotal = 0.0;
				var durationColors = [];
				for(i = 0; i < eventTableInput.length; i++)
				{
					eventRow =  eventTableInput[i];

					if (haveDuration)
					{
						duration = eventRow['duration'];
						durationTotal += duration;
						if (categoriesArray.length && categoryKey.length)
						{
							catIdx = categoriesArray.indexOf(eventRow[categoryKey]);
							console.log('cat: ' + eventRow[categoryKey] + '. cat idx: ' + catIdx + "/" + categoriesArray.length + 'cat color: ' + categoriesColors[catIdx]);
							//durationColor = toColor(categoriesColors[catIdx] * (catIdx / categoriesArray.length));
							durationColor = toColor((catIdx+1) / (categoriesArray.length+1));
						}
						else
						{
							durationColor = toColor(Math.random());
						}
						durationData.push(duration);
						durationColors.push(durationColor);
					}
					
					eventTable += "<tr>"
					for (var key in eventRow)
					{
						if (key == 'duration')
						{
							var x = "<td bgcolor=\"" + durationColor + "\"><font color=\"#FFFFFF\">" + eventRow[key] + "</font></td>";	
							console.log(x);
							eventTable += x;							
						}
						else
						{
							eventTable += "<td>" + eventRow[key] + "</td>";
						}
					}
					eventTable += "</tr>"
				}
      			eventTable += "</table>";
      			eventList.innerHTML = eventTable;
				
				// Draw a pie chart
				if (canvas)
				{
					var ctx = canvas.getContext("2d");
					
					ctx.clearRect(0, 0, canvas.width, canvas.height);
					if (durationData.length)
					{				
						var center = [canvas.width / 2, canvas.height / 2];
						var radius = Math.min(canvas.width, canvas.height) / 2;
						var lastPosition = 0, total = 0;
						
						for (var i = 0; i < durationData.length; i++) 
						{
							ctx.fillStyle = durationColors[i];
							ctx.beginPath();
							ctx.moveTo(center[0],center[1]);
							ctx.arc(center[0],center[1],
									radius,
									lastPosition,lastPosition+(Math.PI*2*(durationData[i]/durationTotal)),
									false);
							ctx.lineTo(center[0],center[1]);
							ctx.fill();
							lastPosition += Math.PI*2*(durationData[i]/durationTotal);
						}
					}
				}
			}

			reader.readAsText(file);
		});
}

// SIG // Begin signature block
// SIG // MIIpKgYJKoZIhvcNAQcCoIIpGzCCKRcCAQExDzANBglg
// SIG // hkgBZQMEAgEFADB3BgorBgEEAYI3AgEEoGkwZzAyBgor
// SIG // BgEEAYI3AgEeMCQCAQEEEBDgyQbOONQRoqMAEEvTUJAC
// SIG // AQACAQACAQACAQACAQAwMTANBglghkgBZQMEAgEFAAQg
// SIG // E2tQzT2eYrC9dof7dXH5GmgVsFHGOZhCORk6k5XApBqg
// SIG // gg4ZMIIGsDCCBJigAwIBAgIQCK1AsmDSnEyfXs2pvZOu
// SIG // 2TANBgkqhkiG9w0BAQwFADBiMQswCQYDVQQGEwJVUzEV
// SIG // MBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
// SIG // d3cuZGlnaWNlcnQuY29tMSEwHwYDVQQDExhEaWdpQ2Vy
// SIG // dCBUcnVzdGVkIFJvb3QgRzQwHhcNMjEwNDI5MDAwMDAw
// SIG // WhcNMzYwNDI4MjM1OTU5WjBpMQswCQYDVQQGEwJVUzEX
// SIG // MBUGA1UEChMORGlnaUNlcnQsIEluYy4xQTA/BgNVBAMT
// SIG // OERpZ2lDZXJ0IFRydXN0ZWQgRzQgQ29kZSBTaWduaW5n
// SIG // IFJTQTQwOTYgU0hBMzg0IDIwMjEgQ0ExMIICIjANBgkq
// SIG // hkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA1bQvQtAorXi3
// SIG // XdU5WRuxiEL1M4zrPYGXcMW7xIUmMJ+kjmjYXPXrNCQH
// SIG // 4UtP03hD9BfXHtr50tVnGlJPDqFX/IiZwZHMgQM+TXAk
// SIG // ZLON4gh9NH1MgFcSa0OamfLFOx/y78tHWhOmTLMBICXz
// SIG // ENOLsvsI8IrgnQnAZaf6mIBJNYc9URnokCF4RS6hnyzh
// SIG // GMIazMXuk0lwQjKP+8bqHPNlaJGiTUyCEUhSaN4QvRRX
// SIG // XegYE2XFf7JPhSxIpFaENdb5LpyqABXRN/4aBpTCfMjq
// SIG // GzLmysL0p6MDDnSlrzm2q2AS4+jWufcx4dyt5Big2MEj
// SIG // R0ezoQ9uo6ttmAaDG7dqZy3SvUQakhCBj7A7CdfHmzJa
// SIG // wv9qYFSLScGT7eG0XOBv6yb5jNWy+TgQ5urOkfW+0/tv
// SIG // k2E0XLyTRSiDNipmKF+wc86LJiUGsoPUXPYVGUztYuBe
// SIG // M/Lo6OwKp7ADK5GyNnm+960IHnWmZcy740hQ83eRGv7b
// SIG // UKJGyGFYmPV8AhY8gyitOYbs1LcNU9D4R+Z1MI3sMJN2
// SIG // FKZbS110YU0/EpF23r9Yy3IQKUHw1cVtJnZoEUETWJrc
// SIG // JisB9IlNWdt4z4FKPkBHX8mBUHOFECMhWWCKZFTBzCEa
// SIG // 6DgZfGYczXg4RTCZT/9jT0y7qg0IU0F8WD1Hs/q27Iwy
// SIG // CQLMbDwMVhECAwEAAaOCAVkwggFVMBIGA1UdEwEB/wQI
// SIG // MAYBAf8CAQAwHQYDVR0OBBYEFGg34Ou2O/hfEYb7/mF7
// SIG // CIhl9E5CMB8GA1UdIwQYMBaAFOzX44LScV1kTN8uZz/n
// SIG // upiuHA9PMA4GA1UdDwEB/wQEAwIBhjATBgNVHSUEDDAK
// SIG // BggrBgEFBQcDAzB3BggrBgEFBQcBAQRrMGkwJAYIKwYB
// SIG // BQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBB
// SIG // BggrBgEFBQcwAoY1aHR0cDovL2NhY2VydHMuZGlnaWNl
// SIG // cnQuY29tL0RpZ2lDZXJ0VHJ1c3RlZFJvb3RHNC5jcnQw
// SIG // QwYDVR0fBDwwOjA4oDagNIYyaHR0cDovL2NybDMuZGln
// SIG // aWNlcnQuY29tL0RpZ2lDZXJ0VHJ1c3RlZFJvb3RHNC5j
// SIG // cmwwHAYDVR0gBBUwEzAHBgVngQwBAzAIBgZngQwBBAEw
// SIG // DQYJKoZIhvcNAQEMBQADggIBADojRD2NCHbuj7w6mdNW
// SIG // 4AIapfhINPMstuZ0ZveUcrEAyq9sMCcTEp6QRJ9L/Z6j
// SIG // fCbVN7w6XUhtldU/SfQnuxaBRVD9nL22heB2fjdxyyL3
// SIG // WqqQz/WTauPrINHVUHmImoqKwba9oUgYftzYgBoRGRjN
// SIG // YZmBVvbJ43bnxOQbX0P4PpT/djk9ntSZz0rdKOtfJqGV
// SIG // WEjVGv7XJz/9kNF2ht0csGBc8w2o7uCJob054ThO2m67
// SIG // Np375SFTWsPK6Wrxoj7bQ7gzyE84FJKZ9d3OVG3ZXQIU
// SIG // H0AzfAPilbLCIXVzUstG2MQ0HKKlS43Nb3Y3LIU/Gs4m
// SIG // 6Ri+kAewQ3+ViCCCcPDMyu/9KTVcH4k4Vfc3iosJocsL
// SIG // 6TEa/y4ZXDlx4b6cpwoG1iZnt5LmTl/eeqxJzy6kdJKt
// SIG // 2zyknIYf48FWGysj/4+16oh7cGvmoLr9Oj9FpsToFpFS
// SIG // i0HASIRLlk2rREDjjfAVKM7t8RhWByovEMQMCGQ8M4+u
// SIG // KIw8y4+ICw2/O/TOHnuO77Xry7fwdxPm5yg/rBKupS8i
// SIG // bEH5glwVZsxsDsrFhsP2JjMMB0ug0wcCampAMEhLNKhR
// SIG // ILutG4UI4lkNbcoFUCvqShyepf2gpx8GdOfy1lKQ/a+F
// SIG // SCH5Vzu0nAPthkX0tGFuv2jiJmCG6sivqf6UHedjGzqG
// SIG // VnhOMIIHYTCCBUmgAwIBAgIQBLBsef3G918UKccZD/VQ
// SIG // JDANBgkqhkiG9w0BAQsFADBpMQswCQYDVQQGEwJVUzEX
// SIG // MBUGA1UEChMORGlnaUNlcnQsIEluYy4xQTA/BgNVBAMT
// SIG // OERpZ2lDZXJ0IFRydXN0ZWQgRzQgQ29kZSBTaWduaW5n
// SIG // IFJTQTQwOTYgU0hBMzg0IDIwMjEgQ0ExMB4XDTI0MDEy
// SIG // OTAwMDAwMFoXDTI1MDEyODIzNTk1OVowaTELMAkGA1UE
// SIG // BhMCVVMxEzARBgNVBAgTCkNhbGlmb3JuaWExEzARBgNV
// SIG // BAcTClNhbiBSYWZhZWwxFzAVBgNVBAoTDkF1dG9kZXNr
// SIG // LCBJbmMuMRcwFQYDVQQDEw5BdXRvZGVzaywgSW5jLjCC
// SIG // AiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAM/J
// SIG // SojUP1kG9c6WHHB8bFmGgw0DesYqMWxwpx5PMb4yDkws
// SIG // t7NmUGF0SdWUCYNxgcPuiV9/u6YwwUErtVLrl82N58b6
// SIG // IZXjNDyeHh/6sHM9oT6IfX8IKgi9vozCPGmJlauDrcE6
// SIG // 3pgCmeTdMx3iiIKmCR+ESaErda7ER8f8vYKieQ2obJdy
// SIG // i3A5NAU1alwNVDY4qPOEsGvWX/KWJJPLIsqI6MWJZxqy
// SIG // /Jtge7uRMEeaZDvUsJKEMw3JfdJcjMVMtC5OeMoALvyV
// SIG // rBUplrv0TpPYuO+5Hw5wTbV0P3clLqNRKdcupOPyEtE2
// SIG // HkofbgjA69S0QOggHI9oTayanr6hJA7B0+LUes2T7577
// SIG // S2s86k4Dbg/YPL75MKf8E5qq0YwPUCUYAVG0lCvVXtdq
// SIG // EwXXJ/ygW6m4QYE/lYPuJTozwSHEu59noUbD8dCD194D
// SIG // fVvfo31oZ2fgJhHlA7xlFN9LhmxaUvAsUdvvZznmdauP
// SIG // tOfrdrbov4EhtrR6gRGKyXFMCW1YE9uwljSYURWYfkvN
// SIG // TIXMG8p+RHjfylgkmahnlnI5W/exbfT7sx/0WzGf+BLd
// SIG // 4U4F5mt9VB3VaOT7xxEZu0I4oSm0u2f4MzxwHzeBJdYp
// SIG // xfU3uj+5j4ueLVaQm4DLiw+cI9Yk/qSLY2I9ENFEA+Tb
// SIG // v6zpqH/1mSYhJbIWQ3WzAgMBAAGjggIDMIIB/zAfBgNV
// SIG // HSMEGDAWgBRoN+Drtjv4XxGG+/5hewiIZfROQjAdBgNV
// SIG // HQ4EFgQUb+nLSdj/EkEoo/c0gxTUAZeqZ6swPgYDVR0g
// SIG // BDcwNTAzBgZngQwBBAEwKTAnBggrBgEFBQcCARYbaHR0
// SIG // cDovL3d3dy5kaWdpY2VydC5jb20vQ1BTMA4GA1UdDwEB
// SIG // /wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzCBtQYD
// SIG // VR0fBIGtMIGqMFOgUaBPhk1odHRwOi8vY3JsMy5kaWdp
// SIG // Y2VydC5jb20vRGlnaUNlcnRUcnVzdGVkRzRDb2RlU2ln
// SIG // bmluZ1JTQTQwOTZTSEEzODQyMDIxQ0ExLmNybDBToFGg
// SIG // T4ZNaHR0cDovL2NybDQuZGlnaWNlcnQuY29tL0RpZ2lD
// SIG // ZXJ0VHJ1c3RlZEc0Q29kZVNpZ25pbmdSU0E0MDk2U0hB
// SIG // Mzg0MjAyMUNBMS5jcmwwgZQGCCsGAQUFBwEBBIGHMIGE
// SIG // MCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2Vy
// SIG // dC5jb20wXAYIKwYBBQUHMAKGUGh0dHA6Ly9jYWNlcnRz
// SIG // LmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydFRydXN0ZWRHNENv
// SIG // ZGVTaWduaW5nUlNBNDA5NlNIQTM4NDIwMjFDQTEuY3J0
// SIG // MAkGA1UdEwQCMAAwDQYJKoZIhvcNAQELBQADggIBAMt5
// SIG // 5kcgD5pZ6n+UtWzjrAhvEm9HAlNDYH9At9EBkM6xPUSS
// SIG // lnOKAs2iCjJ89DyQHJyqS3cq67LN7kX8fxp1gYDhviTq
// SIG // zG28GbIzMdL6bISNPgsB+lkp9OjXwkFDIbrDGL+Bm1Rq
// SIG // fmTZGUKJ7U3byC82hUQJP0V8SWjsUEk7s7BCgyp0X289
// SIG // QDGsDqDTcGjFtMgAiB16Et8S2hgQaLUQb96tO9EXFrha
// SIG // 0yLz/0k9USpTy9zppeZ83iCBkrY0x+wxbr44VYBdi2u+
// SIG // FgWQwivKl0gnwEWvVcTZwnHmpFIbcH4GQZELyKuK72Xl
// SIG // yHnA2TOnLaJrGFGFJm18kdPlDRyjksHJ18myn4YkCaGU
// SIG // Url7nnDvTQsI08QdH1QH4A6+WKjsdRD1lg84ZOzOCyGx
// SIG // 9lSEc5Q3H1m0tMsMzRIgj/quye8BYa9BAh8YYOdwI3ja
// SIG // BZ5xSr6ZlBww5Rih+ebDoVoang7CjTnIzsLUiGSF3f5j
// SIG // qZZa7tRAzldJw2nO5890fn0Yg1k3FMR3dfPGRaOCcZdA
// SIG // idLpGDYKpqo6PwCzDjefLowX+quvs/GGQon/6jDYzKkR
// SIG // kvxKzNpB1WUBmLeJ2khDbg8/jbFvQWcx7kCgRnyn3obj
// SIG // rr17Fyqi3G1HMHZvgFhM5Gqbjd2KcRm6VbFo1E88jTrT
// SIG // KW+N7bETwapyBSJLntZ5MYIaaTCCGmUCAQEwfTBpMQsw
// SIG // CQYDVQQGEwJVUzEXMBUGA1UEChMORGlnaUNlcnQsIElu
// SIG // Yy4xQTA/BgNVBAMTOERpZ2lDZXJ0IFRydXN0ZWQgRzQg
// SIG // Q29kZSBTaWduaW5nIFJTQTQwOTYgU0hBMzg0IDIwMjEg
// SIG // Q0ExAhAEsGx5/cb3XxQpxxkP9VAkMA0GCWCGSAFlAwQC
// SIG // AQUAoHwwEAYKKwYBBAGCNwIBDDECMAAwGQYJKoZIhvcN
// SIG // AQkDMQwGCisGAQQBgjcCAQQwHAYKKwYBBAGCNwIBCzEO
// SIG // MAwGCisGAQQBgjcCARUwLwYJKoZIhvcNAQkEMSIEIEbd
// SIG // WQM2S21TuS79rlwxUSv0YHG/2T9BSOnhKVIW2kJ5MA0G
// SIG // CSqGSIb3DQEBAQUABIICAEeTJUki50KUxZipUsADXrIZ
// SIG // SZO28rojCAUnaiN8Ui/q6DRJJOnu/ARrd2QMLf1sbVJJ
// SIG // lUuBHihF1jhWf5rWXa4kdcHjgsz4xbjNMKmfJcJVAKFl
// SIG // N43muias7Jerucg8DOQsvKdHoSn1IvcCU2HMAUHGjNsQ
// SIG // 1oButZBrNR7ucc0lOBj5lKzi2fhEn/cEhv4JuUo2ZAWs
// SIG // EG/fmwO/S8lSv9hJ7RmN0dgsOWrWrlLPCXUiQ3VM8gZe
// SIG // qSDwk2wfOzLbeEpbfWWBYufsk5SiT8YIPiwWhVhTn0Yh
// SIG // D5TVewo8l4N2BltdV6WKVWPmmYtukxKK03OYzAqjETbj
// SIG // J+5RFY+FAiyDboTdKDN+86u1Nz3smyweibisUDSv61Ht
// SIG // n+yZonC+75TSAUbi8l4leaMWTRIN3SqYLQkwuVp5OQvy
// SIG // Ty0Rls9ro4x9YWRLD0E7pXpylqMhLO3/Hc9i+dwbPIpJ
// SIG // ccOyHrcofxdd2j2eb4IbY8AdEhE887NLkN9gR1UW58IQ
// SIG // Vi0tmIo2C8s0s3Z17fVNkSYK14/dJa/tmYfK9JxISxye
// SIG // nFtUF7OibHc+UspQtfJNF7F+Shf5TUbznwYdTYoZW8Z8
// SIG // 5Si++WipY6jfEOWYz34XOjJliKrV7EC0cIqsDPIEcFIF
// SIG // Da8iOtOVwqBhPPREHtfgVGlDG2eMEWpqwKZid0XP7KuK
// SIG // oYIXPzCCFzsGCisGAQQBgjcDAwExghcrMIIXJwYJKoZI
// SIG // hvcNAQcCoIIXGDCCFxQCAQMxDzANBglghkgBZQMEAgEF
// SIG // ADB3BgsqhkiG9w0BCRABBKBoBGYwZAIBAQYJYIZIAYb9
// SIG // bAcBMDEwDQYJYIZIAWUDBAIBBQAEIIIyztJ9Mg21FgvS
// SIG // 1Uzwvdb5fmQKzeHiZO19DtE4rjWFAhAiS6GTJFcTVvSn
// SIG // 96gQdy02GA8yMDI0MDkxOTE0MzIzMFqgghMJMIIGwjCC
// SIG // BKqgAwIBAgIQBUSv85SdCDmmv9s/X+VhFjANBgkqhkiG
// SIG // 9w0BAQsFADBjMQswCQYDVQQGEwJVUzEXMBUGA1UEChMO
// SIG // RGlnaUNlcnQsIEluYy4xOzA5BgNVBAMTMkRpZ2lDZXJ0
// SIG // IFRydXN0ZWQgRzQgUlNBNDA5NiBTSEEyNTYgVGltZVN0
// SIG // YW1waW5nIENBMB4XDTIzMDcxNDAwMDAwMFoXDTM0MTAx
// SIG // MzIzNTk1OVowSDELMAkGA1UEBhMCVVMxFzAVBgNVBAoT
// SIG // DkRpZ2lDZXJ0LCBJbmMuMSAwHgYDVQQDExdEaWdpQ2Vy
// SIG // dCBUaW1lc3RhbXAgMjAyMzCCAiIwDQYJKoZIhvcNAQEB
// SIG // BQADggIPADCCAgoCggIBAKNTRYcdg45brD5UsyPgz5/X
// SIG // 5dLnXaEOCdwvSKOXejsqnGfcYhVYwamTEafNqrJq3RAp
// SIG // ih5iY2nTWJw1cb86l+uUUI8cIOrHmjsvlmbjaedp/lvD
// SIG // 1isgHMGXlLSlUIHyz8sHpjBoyoNC2vx/CSSUpIIa2mq6
// SIG // 2DvKXd4ZGIX7ReoNYWyd/nFexAaaPPDFLnkPG2ZS48jW
// SIG // Pl/aQ9OE9dDH9kgtXkV1lnX+3RChG4PBuOZSlbVH13gp
// SIG // OWvgeFmX40QrStWVzu8IF+qCZE3/I+PKhu60pCFkcOvV
// SIG // 5aDaY7Mu6QXuqvYk9R28mxyyt1/f8O52fTGZZUdVnUok
// SIG // L6wrl76f5P17cz4y7lI0+9S769SgLDSb495uZBkHNwGR
// SIG // Dxy1Uc2qTGaDiGhiu7xBG3gZbeTZD+BYQfvYsSzhUa+0
// SIG // rRUGFOpiCBPTaR58ZE2dD9/O0V6MqqtQFcmzyrzXxDto
// SIG // RKOlO0L9c33u3Qr/eTQQfqZcClhMAD6FaXXHg2TWdc2P
// SIG // EnZWpST618RrIbroHzSYLzrqawGw9/sqhux7UjipmAmh
// SIG // cbJsca8+uG+W1eEQE/5hRwqM/vC2x9XH3mwk8L9Cgsqg
// SIG // cT2ckpMEtGlwJw1Pt7U20clfCKRwo+wK8REuZODLIivK
// SIG // 8SgTIUlRfgZm0zu++uuRONhRB8qUt+JQofM604qDy0B7
// SIG // AgMBAAGjggGLMIIBhzAOBgNVHQ8BAf8EBAMCB4AwDAYD
// SIG // VR0TAQH/BAIwADAWBgNVHSUBAf8EDDAKBggrBgEFBQcD
// SIG // CDAgBgNVHSAEGTAXMAgGBmeBDAEEAjALBglghkgBhv1s
// SIG // BwEwHwYDVR0jBBgwFoAUuhbZbU2FL3MpdpovdYxqII+e
// SIG // yG8wHQYDVR0OBBYEFKW27xPn783QZKHVVqllMaPe1eNJ
// SIG // MFoGA1UdHwRTMFEwT6BNoEuGSWh0dHA6Ly9jcmwzLmRp
// SIG // Z2ljZXJ0LmNvbS9EaWdpQ2VydFRydXN0ZWRHNFJTQTQw
// SIG // OTZTSEEyNTZUaW1lU3RhbXBpbmdDQS5jcmwwgZAGCCsG
// SIG // AQUFBwEBBIGDMIGAMCQGCCsGAQUFBzABhhhodHRwOi8v
// SIG // b2NzcC5kaWdpY2VydC5jb20wWAYIKwYBBQUHMAKGTGh0
// SIG // dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2Vy
// SIG // dFRydXN0ZWRHNFJTQTQwOTZTSEEyNTZUaW1lU3RhbXBp
// SIG // bmdDQS5jcnQwDQYJKoZIhvcNAQELBQADggIBAIEa1t6g
// SIG // qbWYF7xwjU+KPGic2CX/yyzkzepdIpLsjCICqbjPgKjZ
// SIG // 5+PF7SaCinEvGN1Ott5s1+FgnCvt7T1IjrhrunxdvcJh
// SIG // N2hJd6PrkKoS1yeF844ektrCQDifXcigLiV4JZ0qBXqE
// SIG // KZi2V3mP2yZWK7Dzp703DNiYdk9WuVLCtp04qYHnbUFc
// SIG // jGnRuSvExnvPnPp44pMadqJpddNQ5EQSviANnqlE0Pjl
// SIG // SXcIWiHFtM+YlRpUurm8wWkZus8W8oM3NG6wQSbd3lqX
// SIG // TzON1I13fXVFoaVYJmoDRd7ZULVQjK9WvUzF4UbFKNOt
// SIG // 50MAcN7MmJ4ZiQPq1JE3701S88lgIcRWR+3aEUuMMsOI
// SIG // 5ljitts++V+wQtaP4xeR0arAVeOGv6wnLEHQmjNKqDbU
// SIG // uXKWfpd5OEhfysLcPTLfddY2Z1qJ+Panx+VPNTwAvb6c
// SIG // Kmx5AdzaROY63jg7B145WPR8czFVoIARyxQMfq68/qTr
// SIG // eWWqaNYiyjvrmoI1VygWy2nyMpqy0tg6uLFGhmu6F/3E
// SIG // d2wVbK6rr3M66ElGt9V/zLY4wNjsHPW2obhDLN9OTH0e
// SIG // aHDAdwrUAuBcYLso/zjlUlrWrBciI0707NMX+1Br/wd3
// SIG // H3GXREHJuEbTbDJ8WC9nR2XlG3O2mflrLAZG70Ee8PBf
// SIG // 4NvZrZCARK+AEEGKMIIGrjCCBJagAwIBAgIQBzY3tyRU
// SIG // fNhHrP0oZipeWzANBgkqhkiG9w0BAQsFADBiMQswCQYD
// SIG // VQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkw
// SIG // FwYDVQQLExB3d3cuZGlnaWNlcnQuY29tMSEwHwYDVQQD
// SIG // ExhEaWdpQ2VydCBUcnVzdGVkIFJvb3QgRzQwHhcNMjIw
// SIG // MzIzMDAwMDAwWhcNMzcwMzIyMjM1OTU5WjBjMQswCQYD
// SIG // VQQGEwJVUzEXMBUGA1UEChMORGlnaUNlcnQsIEluYy4x
// SIG // OzA5BgNVBAMTMkRpZ2lDZXJ0IFRydXN0ZWQgRzQgUlNB
// SIG // NDA5NiBTSEEyNTYgVGltZVN0YW1waW5nIENBMIICIjAN
// SIG // BgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAxoY1Bkmz
// SIG // wT1ySVFVxyUDxPKRN6mXUaHW0oPRnkyibaCwzIP5WvYR
// SIG // oUQVQl+kiPNo+n3znIkLf50fng8zH1ATCyZzlm34V6gC
// SIG // ff1DtITaEfFzsbPuK4CEiiIY3+vaPcQXf6sZKz5C3GeO
// SIG // 6lE98NZW1OcoLevTsbV15x8GZY2UKdPZ7Gnf2ZCHRgB7
// SIG // 20RBidx8ald68Dd5n12sy+iEZLRS8nZH92GDGd1ftFQL
// SIG // IWhuNyG7QKxfst5Kfc71ORJn7w6lY2zkpsUdzTYNXNXm
// SIG // G6jBZHRAp8ByxbpOH7G1WE15/tePc5OsLDnipUjW8LAx
// SIG // E6lXKZYnLvWHpo9OdhVVJnCYJn+gGkcgQ+NDY4B7dW4n
// SIG // JZCYOjgRs/b2nuY7W+yB3iIU2YIqx5K/oN7jPqJz+ucf
// SIG // WmyU8lKVEStYdEAoq3NDzt9KoRxrOMUp88qqlnNCaJ+2
// SIG // RrOdOqPVA+C/8KI8ykLcGEh/FDTP0kyr75s9/g64ZCr6
// SIG // dSgkQe1CvwWcZklSUPRR8zZJTYsg0ixXNXkrqPNFYLwj
// SIG // jVj33GHek/45wPmyMKVM1+mYSlg+0wOI/rOP015LdhJR
// SIG // k8mMDDtbiiKowSYI+RQQEgN9XyO7ZONj4KbhPvbCdLI/
// SIG // Hgl27KtdRnXiYKNYCQEoAA6EVO7O6V3IXjASvUaetdN2
// SIG // udIOa5kM0jO0zbECAwEAAaOCAV0wggFZMBIGA1UdEwEB
// SIG // /wQIMAYBAf8CAQAwHQYDVR0OBBYEFLoW2W1NhS9zKXaa
// SIG // L3WMaiCPnshvMB8GA1UdIwQYMBaAFOzX44LScV1kTN8u
// SIG // Zz/nupiuHA9PMA4GA1UdDwEB/wQEAwIBhjATBgNVHSUE
// SIG // DDAKBggrBgEFBQcDCDB3BggrBgEFBQcBAQRrMGkwJAYI
// SIG // KwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNv
// SIG // bTBBBggrBgEFBQcwAoY1aHR0cDovL2NhY2VydHMuZGln
// SIG // aWNlcnQuY29tL0RpZ2lDZXJ0VHJ1c3RlZFJvb3RHNC5j
// SIG // cnQwQwYDVR0fBDwwOjA4oDagNIYyaHR0cDovL2NybDMu
// SIG // ZGlnaWNlcnQuY29tL0RpZ2lDZXJ0VHJ1c3RlZFJvb3RH
// SIG // NC5jcmwwIAYDVR0gBBkwFzAIBgZngQwBBAIwCwYJYIZI
// SIG // AYb9bAcBMA0GCSqGSIb3DQEBCwUAA4ICAQB9WY7Ak7Zv
// SIG // mKlEIgF+ZtbYIULhsBguEE0TzzBTzr8Y+8dQXeJLKftw
// SIG // ig2qKWn8acHPHQfpPmDI2AvlXFvXbYf6hCAlNDFnzbYS
// SIG // lm/EUExiHQwIgqgWvalWzxVzjQEiJc6VaT9Hd/tydBTX
// SIG // /6tPiix6q4XNQ1/tYLaqT5Fmniye4Iqs5f2MvGQmh2yS
// SIG // vZ180HAKfO+ovHVPulr3qRCyXen/KFSJ8NWKcXZl2szw
// SIG // cqMj+sAngkSumScbqyQeJsG33irr9p6xeZmBo1aGqwpF
// SIG // yd/EjaDnmPv7pp1yr8THwcFqcdnGE4AJxLafzYeHJLtP
// SIG // o0m5d2aR8XKc6UsCUqc3fpNTrDsdCEkPlM05et3/JWOZ
// SIG // Jyw9P2un8WbDQc1PtkCbISFA0LcTJM3cHXg65J6t5TRx
// SIG // ktcma+Q4c6umAU+9Pzt4rUyt+8SVe+0KXzM5h0F4ejjp
// SIG // nOHdI/0dKNPH+ejxmF/7K9h+8kaddSweJywm228Vex4Z
// SIG // iza4k9Tm8heZWcpw8De/mADfIBZPJ/tgZxahZrrdVcA6
// SIG // KYawmKAr7ZVBtzrVFZgxtGIJDwq9gdkT/r+k0fNX2bwE
// SIG // +oLeMt8EifAAzV3C+dAjfwAL5HYCJtnwZXZCpimHCUcr
// SIG // 5n8apIUP/JiW9lVUKx+A+sDyDivl1vupL0QVSucTDh3b
// SIG // NzgaoSv27dZ8/DCCBY0wggR1oAMCAQICEA6bGI750C3n
// SIG // 79tQ4ghAGFowDQYJKoZIhvcNAQEMBQAwZTELMAkGA1UE
// SIG // BhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcG
// SIG // A1UECxMQd3d3LmRpZ2ljZXJ0LmNvbTEkMCIGA1UEAxMb
// SIG // RGlnaUNlcnQgQXNzdXJlZCBJRCBSb290IENBMB4XDTIy
// SIG // MDgwMTAwMDAwMFoXDTMxMTEwOTIzNTk1OVowYjELMAkG
// SIG // A1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZ
// SIG // MBcGA1UECxMQd3d3LmRpZ2ljZXJ0LmNvbTEhMB8GA1UE
// SIG // AxMYRGlnaUNlcnQgVHJ1c3RlZCBSb290IEc0MIICIjAN
// SIG // BgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAv+aQc2je
// SIG // u+RdSjwwIjBpM+zCpyUuySE98orYWcLhKac9WKt2ms2u
// SIG // exuEDcQwH/MbpDgW61bGl20dq7J58soR0uRf1gU8Ug9S
// SIG // H8aeFaV+vp+pVxZZVXKvaJNwwrK6dZlqczKU0RBEEC7f
// SIG // gvMHhOZ0O21x4i0MG+4g1ckgHWMpLc7sXk7Ik/ghYZs0
// SIG // 6wXGXuxbGrzryc/NrDRAX7F6Zu53yEioZldXn1RYjgwr
// SIG // t0+nMNlW7sp7XeOtyU9e5TXnMcvak17cjo+A2raRmECQ
// SIG // ecN4x7axxLVqGDgDEI3Y1DekLgV9iPWCPhCRcKtVgkEy
// SIG // 19sEcypukQF8IUzUvK4bA3VdeGbZOjFEmjNAvwjXWkmk
// SIG // wuapoGfdpCe8oU85tRFYF/ckXEaPZPfBaYh2mHY9WV1C
// SIG // doeJl2l6SPDgohIbZpp0yt5LHucOY67m1O+SkjqePdwA
// SIG // 5EUlibaaRBkrfsCUtNJhbesz2cXfSwQAzH0clcOP9yGy
// SIG // shG3u3/y1YxwLEFgqrFjGESVGnZifvaAsPvoZKYz0YkH
// SIG // 4b235kOkGLimdwHhD5QMIR2yVCkliWzlDlJRR3S+Jqy2
// SIG // QXXeeqxfjT/JvNNBERJb5RBQ6zHFynIWIgnffEx1P2Ps
// SIG // IV/EIFFrb7GrhotPwtZFX50g/KEexcCPorF+CiaZ9eRp
// SIG // L5gdLfXZqbId5RsCAwEAAaOCATowggE2MA8GA1UdEwEB
// SIG // /wQFMAMBAf8wHQYDVR0OBBYEFOzX44LScV1kTN8uZz/n
// SIG // upiuHA9PMB8GA1UdIwQYMBaAFEXroq/0ksuCMS1Ri6en
// SIG // IZ3zbcgPMA4GA1UdDwEB/wQEAwIBhjB5BggrBgEFBQcB
// SIG // AQRtMGswJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRp
// SIG // Z2ljZXJ0LmNvbTBDBggrBgEFBQcwAoY3aHR0cDovL2Nh
// SIG // Y2VydHMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0QXNzdXJl
// SIG // ZElEUm9vdENBLmNydDBFBgNVHR8EPjA8MDqgOKA2hjRo
// SIG // dHRwOi8vY3JsMy5kaWdpY2VydC5jb20vRGlnaUNlcnRB
// SIG // c3N1cmVkSURSb290Q0EuY3JsMBEGA1UdIAQKMAgwBgYE
// SIG // VR0gADANBgkqhkiG9w0BAQwFAAOCAQEAcKC/Q1xV5zhf
// SIG // oKN0Gz22Ftf3v1cHvZqsoYcs7IVeqRq7IviHGmlUIu2k
// SIG // iHdtvRoU9BNKei8ttzjv9P+Aufih9/Jy3iS8UgPITtAq
// SIG // 3votVs/59PesMHqai7Je1M/RQ0SbQyHrlnKhSLSZy51P
// SIG // pwYDE3cnRNTnf+hZqPC/Lwum6fI0POz3A8eHqNJMQBk1
// SIG // RmppVLC4oVaO7KTVPeix3P0c2PR3WlxUjG/voVA9/HYJ
// SIG // aISfb8rbII01YBwCA8sgsKxYoA5AY8WYIsGyWfVVa88n
// SIG // q2x2zm8jLfR+cWojayL/ErhULSd+2DrZ8LaHlv1b0Vys
// SIG // GMNNn3O3AamfV6peKOK5lDGCA3YwggNyAgEBMHcwYzEL
// SIG // MAkGA1UEBhMCVVMxFzAVBgNVBAoTDkRpZ2lDZXJ0LCBJ
// SIG // bmMuMTswOQYDVQQDEzJEaWdpQ2VydCBUcnVzdGVkIEc0
// SIG // IFJTQTQwOTYgU0hBMjU2IFRpbWVTdGFtcGluZyBDQQIQ
// SIG // BUSv85SdCDmmv9s/X+VhFjANBglghkgBZQMEAgEFAKCB
// SIG // 0TAaBgkqhkiG9w0BCQMxDQYLKoZIhvcNAQkQAQQwHAYJ
// SIG // KoZIhvcNAQkFMQ8XDTI0MDkxOTE0MzIzMFowKwYLKoZI
// SIG // hvcNAQkQAgwxHDAaMBgwFgQUZvArMsLCyQ+CXc6qisnG
// SIG // Txmcz0AwLwYJKoZIhvcNAQkEMSIEIFeb3xCMD4Y9iJdO
// SIG // h3qZpGzrlVmIQrtQ5Ln7QX5kh9SCMDcGCyqGSIb3DQEJ
// SIG // EAIvMSgwJjAkMCIEINL25G3tdCLM0dRAV2hBNm+CitpV
// SIG // mq4zFq9NGprUDHgoMA0GCSqGSIb3DQEBAQUABIICAJSP
// SIG // qKHKOKLSnq3UJ/Wo0ETJOSiXBrvW6RRfMnCSXhEDqmV3
// SIG // KCMKS7ApR3F23RslTmSiXAjNGqiDKp7r3AwK/htknBss
// SIG // b4bOBeXQtTZ/1OT7NMEAY+yH+l4z1FeJSe8LwhDYLdLa
// SIG // 6yvTagmfZf36b+8BXITnkaRKAUICSCOn+4rDfOpATWwT
// SIG // OLKLkpx+PMlMIIWsuyWsm3CaIBa47V+9k9G4X5b4AjLb
// SIG // UXGn8xHZYpGlkLH7p2XD96op8qU3L82STS6h/rbxfIUl
// SIG // w/jDC45uul85LkUGTBzG93RcvE8eU19S5wvNN2/qYBd6
// SIG // sUngHmGHDen1ctjTmrXMfwP2r2KiF1M3qqkKgk04BPLC
// SIG // joTax4eGB+XFudEg+Z+Zn35OUhSfnxNFZ3jWqoziNCGY
// SIG // q8uj1a091JfdWsIIsbQHAa6tOWlVvc6CoLEZHO3q6Yl2
// SIG // DH5NqKyy4mnrWwOYcRlD86FcfBRcFfbt1UmTjK8BieVk
// SIG // q85OtGS3qVBCRzwMjSju/mnkL/149sPwFjRF7nRTDIqZ
// SIG // GSsdVkhprLCccPqEoVy+dnBewE4B6nKyVg9RP2LulT0D
// SIG // EkLqhNorCMr918At7SMavKp6HrEpCSrD90AOlJJklQZ0
// SIG // ebR1bOmVbfWG/VtnwNPJUtEEBwVBKvuGHyAnDV0tEbXG
// SIG // GmBHkRTZhDMpXCW9lUNL
// SIG // End signature block
